cmake_minimum_required(
    VERSION 3.12
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER_LAUNCHER "ccache")
set(CMAKE_CXX_COMPILER_LAUNCHER "ccache")
set(CMAKE_MESSAGE_LOG_LEVEL TRACE)
# set(CMAKE_GENERATOR "Ninja")

project(
    imgui_bgfx_backend
    VERSION 1.0
    DESCRIPTION "IMGUI backend for bgfx graphics library."
)

add_library(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui_bgfx_backend/)

target_sources(${PROJECT_NAME}
                PUBLIC
                    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_bgfx_backend/backends/imgui_impl_bgfx.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC imgui bgfx bx bimg)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_libs.cmake)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)