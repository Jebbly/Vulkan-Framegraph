cmake_minimum_required(VERSION 3.10)

project(Vulkan-Rendergraph VERSION 0.1
                           DESCRIPTION "Vulkan Lab!"
                           LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

add_subdirectory(External)
add_subdirectory(Source)
add_subdirectory(Apps)
