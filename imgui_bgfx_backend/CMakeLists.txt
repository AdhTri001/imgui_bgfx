add_library(imgui_impl_bgfx_ STATIC ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_bgfx.cpp ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_bgfx.h)
target_include_directories(imgui_impl_bgfx_ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/backends)
target_link_libraries(imgui_impl_bgfx_ PUBLIC imgui bgfx bx bimg)