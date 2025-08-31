# Path Tracer

<img align="center" src="vk_mini_path_tracer/depth_map.jpg">
<i>Depth Map</i>

# Overview

A small, still in development, implementation of a path tracer based on the project [VK Mini Path Tracer](https://github.com/nvpro-samples/vk_mini_path_tracer) from NVIDIA's public repository, using C++, GLSL, Vulkan and NVIDIA's graphics helper libraries.

# Environment Setup

### 1. Dependencies (Vulkan Drivers, Visual Studio and CMAKE)
- [Vulkan Windows SDK](https://vulkan.lunarg.com/sdk/home)
- [Nvidia Vulkan Drivers](https://developer.nvidia.com/vulkan-driver)
- [Visual Studio](https://visualstudio.microsoft.com/downloads/)
- [CMAKE](https://cmake.org/download/)

### 2. Clone (Download)
```bash
## Download the repository
git clone https://github.com/thorgalwulf/path_tracer.git

## Download NVIDIA static libraries for graphics
git clone https://github.com/nvpro-samples/nvpro_core.git 
```

### 3. Build
```bash
## CMAKE GUI
source: .../path_tracer_vulkan/vk_mini_path_tracer 
build: .../path_tracer_vulkan/vk_mini_path_tracer/build
add entry: “Name” = CMAKE_BUILD_TYPE, “Type” = STRING, “Value” = Debug
configure: default x64
configure - generate - open project
```

### 4. Configuration
```bash
## VS Studio
## Set as startup project to avoid 'Access is denied' error
vk_mini_path_tracer_edit.sln
```

### 5. Run
- Open the solution file on VS Studio and then the source files to compile and run main.cpp. 
- The .exe files will be on the bin_x64 debug folder.
- Shader files will be on vk_mini_path_tracer/_edit folder.
```bash
.../path_tracer_vulkan/vk_mini_path_tracer/build/vk_mini_path_tracer_edit.sln 
.../path_tracer_vulkan/bin_x64/Debug/vk_mini_path_tracer__edit.exe
```

### 6. Rebuild
- Need to rebuild every time the shader file (raytrace.comp.glsl) is modified.
```bash
## CMAKE GUI
source: .../path_tracer_vulkan/vk_mini_path_tracer 
build: .../path_tracer_vulkan/vk_mini_path_tracer/build
configure, generate, open project
## Visual Studio
Set vk_mini_path_tracer as startup project
Compile main.cpp
## File Explorer
Run .../path_tracer_vulkan/bin_x64/Debug/vk_mini_path_tracer__edit.exe
See the output .../path_tracer_vulkan/bin_x64/Debug/out.hdr on GIMP
## GIMP
Open out.hdr on GIMP
```

---

# Notes

> <span style="color: gray;">**Note 1:** Try python-cuda. </span>

> <span style="color: gray;">**Note 2:** Study the code-base. </span>



# Repository

```bash
path_tracer_vulkan                                                    Main directory
path_tracer_vulkan/vk_mini_path_tracer/build/vk_mini_path_tracer.sln  Solution file
path_tracer_vulkan/vk_mini_path_tracer/_edit                          main.cpp + shader
path_tracer_vulkan/bin_x64/Debug                                      Executable (.exe) files
```

---
