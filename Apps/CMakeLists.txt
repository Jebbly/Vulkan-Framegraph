function(ADD_APPLICATION EXECUTABLE_NAME)
    add_executable(${EXECUTABLE_NAME} "${EXECUTABLE_NAME}.cpp")
    target_link_libraries(${EXECUTABLE_NAME} slang glm glfw Vulkan::Vulkan GraphicsCore) # TODO: clean up dependencies
    target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/Source) # TODO: better separation of source and include dirs?
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "Apps")
endfunction()

add_application(Sandbox)
add_application(HelloWorldCompute)
