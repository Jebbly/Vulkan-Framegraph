cmake_minimum_required(VERSION 3.10)

project(Vulkan-Rendergraph VERSION 0.1
                           DESCRIPTION "Vulkan Lab!"
                           LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(External)

find_package(Vulkan REQUIRED)

add_subdirectory(Source)
add_subdirectory(Apps)
