set(GRAPHICS_CORE_SRC
    Command.cpp
    Context.cpp
    Device.cpp
    Instance.cpp
    Resources.cpp
    Swapchain.cpp
    Synchronization.cpp
    Window.cpp
)

add_library(GraphicsCore STATIC ${GRAPHICS_CORE_SRC})

# TODO: most libraries linked here should be private.
target_link_libraries(GraphicsCore glfw Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)
