cmake_minimum_required(VERSION 3.7)

project(vsgRenderSandBox
        VERSION 0.0.0
        DESCRIPTION "Template of how to create a program using VulkanSceneGraph and CMake"
        LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
include(cmake/conan_config.cmake)


# Change the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)


find_package(vsg 1.0.0 CONFIG REQUIRED)
find_package(vsgXchange 1.0.0 QUIET)


add_subdirectory(src/sample)
add_subdirectory(src/renderHead)