set(GRAPHICS_CORE_SRC
    Command.cpp
    Context.cpp
    Device.cpp
    Instance.cpp
    Swapchain.cpp
    Window.cpp
)

add_library(GraphicsCore STATIC ${GRAPHICS_CORE_SRC})
target_link_libraries(GraphicsCore glfw Vulkan::Vulkan)
