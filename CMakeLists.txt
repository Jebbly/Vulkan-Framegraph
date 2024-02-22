cmake_minimum_required(VERSION 3.10)

project(Vulkan-Rendergraph VERSION 0.1
                           DESCRIPTION "Vulkan Lab!"
                           LANGUAGES CXX)

find_package(Vulkan REQUIRED)

add_subdirectory(External)
add_subdirectory(Source)
