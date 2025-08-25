// Path tracers trace paths of light through scenes to render images. Most path tracers today use geometric optics, which assumes that light travels along rays. 
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>				// will be used to save the image to a Radiance High Dynamic Range (HDR) file 
#include <nvvk/context_vk.hpp>
#include <nvvk/resourceallocator_vk.hpp>	// For NVVK memory allocators
#include <nvvk/error_vk.hpp>				// For NVVK_CHECK
#include <nvh/fileoperations.hpp>			// For nvh::loadFile
#include <nvvk/shaders_vk.hpp>				// For nvvk::createShaderModule

// Constants for the width and height of the image to be rendered.
static const uint64_t render_width = 800; 
static const uint64_t render_height = 600;  

// Workgroup size for the compute shader
static const uint32_t workgroup_width = 16;
static const uint32_t workgroup_height = 8;

int main(int argc, const char** argv)
{	
	// Create the Vulkan context
	nvvk::ContextCreateInfo deviceInfo;  // Configuration: Vulkan, GPU, ... settings
	deviceInfo.apiMajor = 1;             // Specify the version of Vulkan
	deviceInfo.apiMinor = 2;

	// Required by VK_KHR_ray_query; allows work to be offloaded onto background threads and parallelized
	deviceInfo.addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME); 
	VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
	deviceInfo.addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, false, &asFeatures);
	
	nvvk::Context context;               // Encapsulates device state in a single object
	context.init(deviceInfo);            // Initialize the context

	/*
    Initialize an NVVK GPU memory allocator;
    Allocate a VkBuffer to hold color data on the GPU;
    Map the (uninitialized) VkBuffer data back to the CPU and print out the first few values.

	Because of the latency of the data transfer between the CPU and GPU, instead of potentially sending data between the CPU and GPU with every command, 
	Vulkan's design encourages applications to batch collections of operations in command buffers, then submit these larger packets of information to the GPU all at once. 
	The GPU can then process these command buffers asynchronously, allowing the CPU to continue executing other tasks in parallel.

	Create a buffer on the GPU to hold the color data
	A VkBuffer (Vulkan buffer) is like a pointer to an array of bytes in C++
	The buffer will contain a float for each of the red, green, and blue channels for each of the render_width x render_height pixels in the image.
	Each float is 4 bytes, so the total size of the buffer in bytes is render_width * render_height * 3 * sizeof(float).
	
	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT means that the buffer can be used as a storage buffer and as a destination for transfer operations
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT means that the CPU can read this buffer's memory.
	VK_MEMORY_PROPERTY_HOST_CACHED_BIT means that the CPU caches this memory.
	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT means that the CPU side of cache management
	is handled automatically, with potentially slower reads/writes.

	Allocate GPU memory using nvvk::ResourceAllocatorDedicated;
	Create a VkBuffer representing an image;
	Map data from the GPU to the CPU;
	Create a command buffer and a command pool;
	Record and submit a command buffer with a vkCmdFill command.
	*/

	// Create the allocator
	nvvk::ResourceAllocatorDedicated allocator;
	allocator.init(context, context.m_physicalDevice);

	// Create the buffer
	VkDeviceSize bufferSizeBytes = render_width * render_height * 3 * sizeof(float);
	VkBufferCreateInfo bufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = bufferSizeBytes,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
	};
	nvvk::Buffer buffer = allocator.createBuffer
	(
		bufferCreateInfo,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
	);

	/* A command buffer (VkCommandBuffer) holds a buffer of commands. You might think of this as a list of instructions for the GPU to do,
	such as “fill a buffer with a value”, “copy an image”, “resize an image”, “build a ray tracing acceleration structure”, “bind a pipeline”, or “trace rays”. */

	// Create the command pool
	VkCommandPoolCreateInfo cmdPoolInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = context.m_queueGCT.familyIndex // Graphics, Compute, Transfer queue
	};
	VkCommandPool cmdPool;
	NVVK_CHECK(vkCreateCommandPool(context, &cmdPoolInfo, nullptr, &cmdPool));

	// Allocate a command buffer from the command pool
	VkCommandBufferAllocateInfo cmdAllocInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = cmdPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	VkCommandBuffer cmdBuffer;
	NVVK_CHECK(vkAllocateCommandBuffers(context, &cmdAllocInfo, &cmdBuffer));

	// Begin recording the command buffer
	VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
									   .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
	NVVK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

	// Fill the buffer: record a command for the GPU to fill the VkBuffer
	const float fillValue = 0.5f;
	const uint32_t& fillValueU32 = reinterpret_cast<const uint32_t&>(fillValue);
	vkCmdFillBuffer(cmdBuffer, buffer.buffer, 0, bufferSizeBytes, fillValueU32);

	/* Add a command that says "Make it so that memory writes by the vkCmdFillBuffer call
	are available to read from the CPU." (In other words, "Flush the GPU caches
	so the CPU can read the data.") To do this, we use a memory barrier.
	Pipeline barriers synchronize memory, and they are one type of thing that makes it so 
	that pipeline stages (such as transfers, shaders, or the CPU) can wait for other 
	pipeline stages' memory accesses and read what those pipeline stages have completed writing. */

	// Memory Barrier
	VkMemoryBarrier memoryBarrier
	{	.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,	// Make transfer writes
		.dstAccessMask = VK_ACCESS_HOST_READ_BIT        // Readable by the CPU
	};
	vkCmdPipelineBarrier
	(   cmdBuffer,												 // The command buffer
		VK_PIPELINE_STAGE_TRANSFER_BIT,                          // From the transfer stage
		VK_PIPELINE_STAGE_HOST_BIT,                              // To the CPU
		0,                                                       // No special flags
		1, &memoryBarrier,                                       // An array of memory barriers
		0, nullptr, 0, nullptr                                   // No other barriers
	);

	// End recording pf the command buffer
	NVVK_CHECK(vkEndCommandBuffer(cmdBuffer));

	// Submit the command buffer so that the GPU executes the commands
	VkSubmitInfo submitInfo
	{ 
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,  
		.commandBufferCount = 1,                              
		.pCommandBuffers = &cmdBuffer 
	};
	NVVK_CHECK(vkQueueSubmit(context.m_queueGCT, 1, &submitInfo, VK_NULL_HANDLE));

	// Synchronization: make the CPU wait for the GPU to finish all the work on a queue
	NVVK_CHECK(vkQueueWaitIdle(context.m_queueGCT));

	// Get the image data back from the GPU
	void* data = allocator.map(buffer);
	stbi_write_hdr("out.hdr", render_width, render_height, 3, reinterpret_cast<float*>(data)); // write the image to a Radiance HDR file in the same dir as the .sln file
	allocator.unmap(buffer);

	// Shaders are like functions that a GPU can run.
	// GLSL -> SPV code: glslangValidator.exe --target-env vulkan1.2 -o shaders/raytrace.comp.glsl.spv shaders/raytrace.comp.glsl
	// vk_mini_path_tracer\_edit\shaders\raytrace.comp.glsl
	/*The CMake build system will do this automatically for any GLSL files in the project. 
	file(GLOB_RECURSE GLSL_SOURCE_FILES "shaders/*.glsl")
	foreach(GLSL ${GLSL_SOURCE_FILES})
		get_filename_component(FILE_NAME ${GLSL} NAME)
		_compile_GLSL(${GLSL} "shaders/${FILE_NAME}.spv" GLSL_SOURCES SPV_OUTPUT)
	endforeach(GLSL)
	(However, one does need to re-run CMake in order for it to detect new shader files.) */

	// Clean up resources
	vkFreeCommandBuffers(context, cmdPool, 1, &cmdBuffer);
	vkDestroyCommandPool(context, cmdPool, nullptr);
	allocator.destroy(buffer);
	allocator.deinit();
	context.deinit();     
}
