# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(GLFW)

# GLM
add_subdirectory(GLM)

# Slang
option(SLANG_ENABLE_GFX OFF)
option(SLANG_ENABLE_SLANGD OFF)
option(SLANG_ENABLE_SLANGRT OFF)
option(SLANG_ENABLE_TESTS OFF)
option(SLANG_ENABLE_EXAMPLES OFF)
set(SLANG_SLANG_LLVM_FLAVOR DISABLE)
set(SLANG_LIB_TYPE STATIC)
add_subdirectory(Slang)

# Spdlog
add_subdirectory(Spdlog)

# VulkanMemoryAllocator
option(VMA_BUILD_DOCUMENTATION OFF)
option(VMA_BUILD_SAMPLES OFF)
add_subdirectory(VulkanMemoryAllocator)

# Cleanup the same way as Slang:
# for this directory and all subdirectories, prepend
# `external/` to the IDE FOLDER property to every target
function(make_external dir)
get_property(external_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
foreach(external_target ${external_targets})
    get_property(folder TARGET ${external_target} PROPERTY FOLDER)
    set_property(
        TARGET ${external_target}
        PROPERTY FOLDER "External/${folder}"
    )
endforeach()

get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
foreach(subdir ${subdirs})
    make_external(${subdir})
endforeach()
endfunction()
make_external(.)
