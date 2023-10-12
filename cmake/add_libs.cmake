#   GLFW
set(glfw_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/glfw)
set(GLFW_BULID_DOCS OFF)
set(GLFW_INSTALL    OFF)
add_subdirectory(${glfw_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_include_directories(${PROJECT_NAME} PUBLIC ${glfw_SOURCE_DIR}/include)


#   BGFX
set(bgfx_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/bgfx.cmake)
set(BGFX_CONFIG_DEBUG     OFF)
set(BGFX_AMALGAMATED      OFF)
set(BGFX_BUILD_EXAMPLES   OFF)
set(BGFX_INSTALL          OFF)
set(BGFX_CUSTOM_TARGETS   OFF)
set(BGFX_INSTALL_EXAMPLES OFF)
set(BGFX_USE_DEBUG_SUFFIX OFF)
set(BGFX_USE_OVR          OFF)
set(BUILD_SHARED_LIBS     OFF)
set(BX_AMALGAMATED        OFF)
add_subdirectory(${bgfx_SOURCE_DIR})
# if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#     target_compile_definitions(${PROJECT_NAME} PUBLIC "BX_CONFIG_DEBUG=1")
# else()
#     target_compile_definitions(${PROJECT_NAME} PUBLIC "BX_CONFIG_DEBUG=0")
# endif()
target_include_directories(${PROJECT_NAME} PUBLIC 
${bgfx_SOURCE_DIR}/bx/include
    ${bgfx_SOURCE_DIR}/bx/include/compat/mingw
    ${bgfx_SOURCE_DIR}/bimg/include
    ${bgfx_SOURCE_DIR}/bgfx/include
    ${bgfx_SOURCE_DIR}/bgfx/examples/common
)
target_compile_options(bimg PUBLIC -mssse3)
target_link_libraries(${PROJECT_NAME} PUBLIC bgfx bimg bx)


#   IMGUI
set(imgui_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/imgui)
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/imstb_rectpack.h
    ${imgui_SOURCE_DIR}/imstb_textedit.h
    ${imgui_SOURCE_DIR}/imstb_truetype.h
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_internal.h
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
)
add_library(imgui_impl_glfw STATIC
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
)
target_link_libraries(imgui_impl_glfw PUBLIC imgui glfw)
target_compile_definitions(imgui PUBLIC -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui imgui_impl_glfw)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/bgfx_imgui_backend
    ${imgui_SOURCE_DIR}
)