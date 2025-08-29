#version 460

layout(local_size_x = 16, local_size_y = 8, local_size_z = 1) in;

layout(binding = 0, set = 0) buffer storageBuffer
{
  float imageData[];
};

void main()
{
  // The resolution of the buffer, which in this case is a hardcoded vector
  // of 2 unsigned integers:
  const uvec2 resolution = uvec2(800, 600);

  // gl_GlobalInvocationID is a uvec3 (vector of three unsigned ints) that contains the index of each invocation in the entire dispatch. 
  // (In this case, this is the index of the pixel in the image.) 
  // Get the coordinates of the pixel for this invocation:
  //
  // .-------.-> x
  // |       |
  // |       |
  // '-------'
  // v
  // y
  const uvec2 pixel = gl_GlobalInvocationID.xy; // store the variables of the pixels

  // If the pixel is outside of the image, do nothing:
  if((pixel.x >= resolution.x) || (pixel.y >= resolution.y))
  {
    return;
  }

  // Create a vector of 3 floats with a different color per pixel.
  const vec3 pixelColor = vec3(float(pixel.x) / resolution.x,  // Red
                               float(pixel.y) / resolution.y,  // Green
                               0.0);                           // Blue
  // Get the index of this invocation in the buffer:
  uint linearIndex = resolution.x * pixel.y + pixel.x;
  // Write the color to the buffer.
  imageData[3 * linearIndex + 0] = pixelColor.r;
  imageData[3 * linearIndex + 1] = pixelColor.g;
  imageData[3 * linearIndex + 2] = pixelColor.b;
}