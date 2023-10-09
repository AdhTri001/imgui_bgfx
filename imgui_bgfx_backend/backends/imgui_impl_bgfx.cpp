#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/timer.h>

#include "vs_ocornut_imgui.bin.h"
#include "fs_ocornut_imgui.bin.h"
#include "vs_imgui_image.bin.h"
#include "fs_imgui_image.bin.h"
#include "imgui_impl_bgfx.h"
#include "imgui_internal.h"

static const bgfx::EmbeddedShader s_embeddedShaders[] =
{
	BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
	BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
	BGFX_EMBEDDED_SHADER(vs_imgui_image),
	BGFX_EMBEDDED_SHADER(fs_imgui_image),

	BGFX_EMBEDDED_SHADER_END()
};

struct ImGui_Implbgfx_Data
{
    uint8_t viewId;
    bx::AllocatorI* allocator;
    bgfx::VertexLayout layout;
    bgfx::ProgramHandle IM_Program;
    bgfx::ProgramHandle IM_ImageProgram;
    bgfx::TextureHandle fontTexture;
    bgfx::UniformHandle u_imageLodEnabled;
    bgfx::UniformHandle s_tex;

    ImGui_Implbgfx_Data() { memset((void*)this, 0, sizeof(*this)); }
};

inline static ImGui_Implbgfx_Data* ImGui_Implbgfx_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_Implbgfx_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

static void* memAlloc(size_t _size, void* _userData)
{
	BX_UNUSED(_userData);
	return bx::alloc(ImGui_Implbgfx_GetBackendData()->allocator, _size);
}

static void memFree(void* _ptr, void* _userData)
{
	BX_UNUSED(_userData);
	bx::free(ImGui_Implbgfx_GetBackendData()->allocator, _ptr);
}

bool ImGui_Implbgfx_Init(int viewId, bx::AllocatorI* allocator) {
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    ImGui_Implbgfx_Data* bd = IM_NEW(ImGui_Implbgfx_Data)();
    bd->viewId = (uint8_t)(viewId & 0xff);
    bd->allocator = allocator;
    bgfx::setViewName(viewId, "imgui_impl_bgfx"); // Helpfull while debugging
    bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);

    io.BackendRendererName = "imgui_impl_bgfx";
    io.BackendRendererUserData = (void*)bd;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bd->IM_Program = bgfx::createProgram(
        bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_ocornut_imgui")
        , bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_ocornut_imgui")
        , true
    );
    bd->u_imageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
    bd->IM_ImageProgram = bgfx::createProgram(
        bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_image")
        , bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_imgui_image")
        , true
    );
    bd->layout
        .begin()
        .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
        .end();
    bd->s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    bd->fontTexture = bgfx::createTexture2D(
        (uint16_t)width
        , (uint16_t)height
        , false
        , 1
        , bgfx::TextureFormat::BGRA8
        , 0
        , bgfx::copy(pixels, width*height*4)
    );

    return true;
}

void ImGui_Implbgfx_Shutdown() {
    ImGui_Implbgfx_Data* bd = ImGui_Implbgfx_GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    
    bgfx::destroy(bd->s_tex);
    bgfx::destroy(bd->u_imageLodEnabled);
    bgfx::destroy(bd->fontTexture);
    bgfx::destroy(bd->IM_ImageProgram);
    bgfx::destroy(bd->IM_Program);
    bd->allocator = NULL;

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    IM_DELETE(bd);

    return true;
}

void ImGui_Implbgfx_NewFrame() {
    ImGui_Implbgfx_Data* bd = ImGui_Implbgfx_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplOpenGL3_Init()?");

    if(isValid(bd->IM_Program))
        ImGui_Implbgfx_CreateDeviceObjects();
}

bool ImGui_Implbgfx_CreateDeviceObjects() {
}

void ImGui_Implbgfx_RenderDrawLists(struct ImDrawData* draw_data) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;
}