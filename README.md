# Path Tracer with Vulkan

From the project: [vk_mini_path_tracer](https://github.com/nvpro-samples/vk_mini_path_tracer)

---

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

## Optional (delete later):
## Leave only _edit, CMAKELists.txt, .gitignore and README.md
## Configure CMAKELists.txt below
git clone https://github.com/nvpro-samples/vk_mini_path_tracer.git 
```

### 3. Build
```bash
## CMAKE GUI
source: C:/.../path_tracer_vulkan/vk_mini_path_tracer 
build: C:/.../path_tracer_vulkan/vk_mini_path_tracer/build
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
- Shader files will be on vk_mini_path_tracer\_edit folder.
```bash
C:\...\vk_mini_path_tracer/build/vk_mini_path_tracer_edit.sln 
C:\...\path_tracer_vulkan\bin_x64\Debug>vk_mini_path_tracer__edit.exe
```

### 6. Rebuild
- Need to rebuild every time the shader file (raytrace.comp.glsl) is modified.
```bash
## CMAKE GUI
source: C:/.../path_tracer_vulkan/vk_mini_path_tracer 
build: C:/.../path_tracer_vulkan/vk_mini_path_tracer/build
configure - generate - open project
## Visual Studio
Set vk_mini_path_tracer as startup project
Compile main.cpp
## File Explorer
Run C:\...\path_tracer_vulkan\bin_x64\Debug>vk_mini_path_tracer__edit.exe
See the output C:\...\path_tracer_vulkan\bin_x64\Debug\out.hdr on GIMP
## GIMP
Open out.hdr on GIMP
```

---

# Notes

> <span style="color: gray;">**Important:**</span>
Try python-cuda.

## CMakeLists.txt
```bash
# Copyright 2020-2024 NVIDIA Corporation
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.5)
project(vk_mini_path_tracer)

#####################################################################################
# look for nvpro_core 1) as a sub-folder 2) at some other locations
# this cannot be put anywhere else since we still didn't find setup.cmake yet
#
if(NOT BASE_DIRECTORY)

  find_path(BASE_DIRECTORY
    NAMES nvpro_core/cmake/setup.cmake
    PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. 
    REQUIRED
    DOC "Directory containing nvpro_core"
    )
endif()
if(EXISTS ${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
  include(${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
else()
  message(FATAL_ERROR "could not find base directory, please set BASE_DIRECTORY to folder containing nvpro_core")
endif()

#####################################################################################
# Add sample packages
#
_add_package_VulkanSDK()
_add_nvpro_core_lib()

#####################################################################################
# Add chapters
add_subdirectory(_edit) # Empty starting project
```

# Repository

```
path_tracer_vulkan                                                    Main directory

[CMAKE]
path_tracer_vulkan\vk_mini_path_tracer\build\vk_mini_path_tracer.sln  Solution file
path_tracer_vulkan\vk_mini_path_tracer\_edit                          main.cpp / shader files / CMakeLists.txt
path_tracer_vulkan\bin_x64\Debug                                      Executable (.exe) files
```

---
