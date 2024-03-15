# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(GLFW)

# GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE GLM)

# Slang
option(SLANG_ENABLE_GFX OFF)
option(SLANG_ENABLE_SLANGD OFF)
option(SLANG_ENABLE_SLANGRT OFF)
option(SLANG_ENABLE_TESTS OFF)
option(SLANG_ENABLE_EXAMPLES OFF)
set(SLANG_SLANG_LLVM_FLAVOR DISABLE)
set(SLANG_LIB_TYPE STATIC)
add_subdirectory(Slang)

# VulkanMemoryAllocator
option(VMA_BUILD_DOCUMENTATION OFF)
option(VMA_BUILD_SAMPLES OFF)
add_subdirectory(VulkanMemoryAllocator)
