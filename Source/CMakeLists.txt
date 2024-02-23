add_subdirectory(GraphicsCore)

add_executable(${PROJECT_NAME} Main.cpp)
target_link_libraries(${PROJECT_NAME} slang glm glfw Vulkan::Vulkan GraphicsCore)
