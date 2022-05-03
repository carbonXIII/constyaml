cmake_minimum_required(VERSION 3.19)

project(constyml)

add_compile_options(-fmodules-ts -freport-bug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(constyaml)
add_subdirectory(test)
add_subdirectory(samples)
