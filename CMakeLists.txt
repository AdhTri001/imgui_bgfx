cmake_minimum_required(
    VERSION 3.12
)

project(
    imgui_bgfx_backend
    VERSION 1.0
    DESCRIPTION "IMGUI backend for bgfx graphics library."
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_libs.cmake)

add_subdirectory(imgui_bgfx_backend)