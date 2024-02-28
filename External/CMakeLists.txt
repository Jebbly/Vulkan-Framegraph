# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(GLFW)

# GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE GLM)

# Slang
add_library(slang INTERFACE)
target_include_directories(slang INTERFACE Slang)

# VulkanMemoryAllocator
option(VMA_BUILD_DOCUMENTATION OFF)
option(VMA_BUILD_SAMPLES OFF)
add_subdirectory(VulkanMemoryAllocator)
