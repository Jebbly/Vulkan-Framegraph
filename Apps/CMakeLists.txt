add_executable(Sandbox Sandbox.cpp)
target_link_libraries(Sandbox slang glm glfw Vulkan::Vulkan GraphicsCore) # TODO: clean up dependencies
target_include_directories(Sandbox PUBLIC ${CMAKE_SOURCE_DIR}/Source) # TODO: better separation of source and include dirs?

# TODO: use a function to add executables
add_executable(HelloWorldCompute HelloWorldCompute.cpp)
target_link_libraries(HelloWorldCompute slang glm glfw Vulkan::Vulkan GraphicsCore)
target_include_directories(HelloWorldCompute PUBLIC ${CMAKE_SOURCE_DIR}/Source)
