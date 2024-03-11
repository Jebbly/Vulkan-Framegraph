add_executable(Sandbox Sandbox.cpp)
target_link_libraries(Sandbox slang glm glfw Vulkan::Vulkan GraphicsCore)
target_include_directories(Sandbox PUBLIC ${CMAKE_SOURCE_DIR}/Source) # TODO: better separation of source and include dirs?
