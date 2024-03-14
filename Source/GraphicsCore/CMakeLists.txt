set(GRAPHICS_CORE_SRC
    Command.cpp
    Context.cpp
    Device.cpp
    Instance.cpp
    Parameters.cpp
    Resources.cpp
    Shader.cpp
    Swapchain.cpp
    Synchronization.cpp
    Window.cpp
)

add_library(GraphicsCore STATIC ${GRAPHICS_CORE_SRC})

# TODO: most libraries linked here should be private, but it's not clear which ones yet.
target_link_libraries(GraphicsCore glfw Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator slang)
