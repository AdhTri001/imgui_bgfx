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

static void glfw_errorCallback(int error, const char *description)
{
	fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

// struct PosColVert {
//     float x, y, z;
//     uint32_t rgba;

//     static void init() {
//         layout.begin()
//             .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
//             .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
//             .end();
//     };
//     static bgfx::VertexLayout layout;
// };

// bgfx::VertexLayout PosColVert::layout;

// PosColVert verts[] = {
//     { 0.5, -1.73205/4, 0., 0xff00ffff},
//     {-0.5, -1.73205/4, 0., 0xff00ffff},
//     { 0. ,  1.73205/4, 0., 0xff00ffff}
// };

uint16_t indices[] = {0,1,2};

int main(int argc, char **argv)
{
	// Create a GLFW window without an OpenGL context.
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
	if (!bgfx::init(init))
		return 1;
	// Set view 0 to the same dimensions as the window and to clear the color buffer.
	const bgfx::ViewId mainView = 0;
	bgfx::setViewClear(mainView,
        BGFX_CLEAR_COLOR);
	bgfx::setViewRect(mainView, 0, 0, bgfx::BackbufferRatio::Equal);
    auto time_start = std::chrono::high_resolution_clock::now();
    int nFrames = 0, FPS = 0;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		// Handle window resize.
		int oldWidth = width, oldHeight = height;
        glfwGetWindowSize(window, &width, &height);
		if (width != oldWidth || height != oldHeight) {
			bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
			bgfx::setViewRect(mainView, 0, 0, bgfx::BackbufferRatio::Equal);
		}
        bgfx::touch(mainView);
        // bgfx::setViewClear(mainView,
        //     BGFX_CLEAR_COLOR,
        //     0xdab7f9ff, 1.0f, 0);
        bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats.");
		bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");
		bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
		bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");
		const bgfx::Stats* stats = bgfx::getStats();
		bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters.", stats->width, stats->height, stats->textWidth, stats->textHeight);
        bgfx::dbgTextPrintf(0, 1, 0x0f, "Framerate: %dfps", FPS);
        // bgfx::setVertexBuffer(0, vbh);
        // bgfx::setIndexBuffer(ibh);
        // bgfx::submit(mainView, program, 0, false);
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