#include <stdio.h>
#include "bx/bx.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "GLFW/glfw3.h"
#if BX_PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include "GLFW/glfw3native.h"
#include <chrono>
#include "./vert.h"
#include "./frag.h"

static void glfw_errorCallback(int error, const char *description)
{
	fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

struct PosColVert {
    float x, y, z;
    uint32_t rgba;

    static void init() {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
            .end();
    };
    static bgfx::VertexLayout layout;
};

bgfx::VertexLayout PosColVert::layout;

PosColVert verts[] = {
    { 1. , 0.       ,0.,0xff00ffff},
    {-0.5, 1.73205/2,0.,0xff00ffff},
    {-0.5,-1.73205/2,0.,0xff00ffff}
};

uint16_t indices[] = {0,1,2};

int main(int argc, char **argv)
{
	if (!glfwInit())
		return 1;
	glfwSetErrorCallback(glfw_errorCallback);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow *window = glfwCreateWindow(1024, 768, "FirstTriangle", nullptr, nullptr);
	if (!window)
		return 1;
	bgfx::renderFrame();
	bgfx::Init init;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	init.platformData.ndt = glfwGetX11Display();
	init.platformData.nwh = (void*)(uintptr_t)glfwGetX11Window(window);
#elif BX_PLATFORM_OSX
	init.platformData.nwh = glfwGetCocoaWindow(window);
#elif BX_PLATFORM_WINDOWS
	init.platformData.nwh = glfwGetWin32Window(window);
#endif
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	init.resolution.width = (uint32_t)width;
	init.resolution.height = (uint32_t)height;
	init.resolution.reset = BGFX_RESET_VSYNC;
    init.type = bgfx::RendererType::Direct3D11;
	if (!bgfx::init(init))
		return 1;
	const bgfx::ViewId mainView = 0;
    bgfx::setDebug(BGFX_DEBUG_TEXT);
    bgfx::setState(BGFX_STATE_DEFAULT);
	bgfx::setViewClear(mainView, BGFX_CLEAR_COLOR);
	bgfx::setViewRect(mainView, 0, 0, bgfx::BackbufferRatio::Equal);
    PosColVert::init();
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(
        bgfx::makeRef((void*) verts, sizeof(verts)), PosColVert::layout
    );
    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(
        bgfx::makeRef((void*) indices, sizeof(indices))
    );
    bgfx::ShaderHandle vertHandle = bgfx::createShader(
        bgfx::makeRef((void*) vertSh, sizeof(vertSh))
    );
    bgfx::ShaderHandle fragHandle = bgfx::createShader(
        bgfx::makeRef((void*) fragSh, sizeof(fragSh))
    );
    bgfx::setViewClear(mainView, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x00ff00ff);
    auto program = bgfx::createProgram(vertHandle, fragHandle, false);
    printf("%d, %d, %d", program.idx, vertHandle.idx, fragHandle.idx);
    auto time_start = std::chrono::high_resolution_clock::now();
    int nFrames = 0, FPS = 0;
	while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        int oldWidth = width, oldHeight = height;
        glfwGetWindowSize(window, &width, &height);
        if (width != oldWidth || height != oldHeight) {
            bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
            bgfx::setViewRect(mainView, 0, 0, bgfx::BackbufferRatio::Equal);
        };
        bgfx::touch(mainView);
        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 1, 0x0f, "Framerate: %dfps", FPS);
        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);
        bgfx::submit(mainView, program, 0, false);
        bgfx::frame();
        nFrames++;
        auto time_now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_start).count();
        if (!FPS | (duration >= 1000)) {
            FPS = nFrames * 1000 / duration;
            time_start = time_now;
            nFrames = 0;
        }
	}
	bgfx::shutdown();
	glfwTerminate();
	return 0;
}