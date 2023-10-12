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

static inline ImGui_Implbgfx_Data* ImGui_Implbgfx_GetBackendData()
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

/**
 * Returns true if both internal transient index and vertex buffer have
 * enough space.
 * @param[in] _numVertices Number of vertices.
 * @param[in] _layout Vertex layout.
 * @param[in] _numIndices Number of indices. */
static inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexLayout& _layout, uint32_t _numIndices)
{
	return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _layout)
		&& (0 == _numIndices || _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices));
}

bool ImGui_Implbgfx_Init(int viewId, bx::AllocatorI* allocator) {
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    ImGui_Implbgfx_Data* bd = IM_NEW(ImGui_Implbgfx_Data)();
    bd->viewId = (uint8_t)(viewId & 0xff);
    if (NULL == allocator)
    {
        static bx::DefaultAllocator _allocator;
        bd->allocator = &_allocator;
    } else {
        bd->allocator = allocator;
    }
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
}

void ImGui_Implbgfx_NewFrame() {
    ImGui_Implbgfx_Data* bd = ImGui_Implbgfx_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplOpenGL3_Init()?");

    if(isValid(bd->IM_Program))
        ImGui_Implbgfx_CreateDeviceObjects();
}

bool ImGui_Implbgfx_CreateDeviceObjects() {
    return true;
}

void ImGui_Implbgfx_RenderDrawLists(struct ImDrawData* draw_data) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    ImGui_Implbgfx_Data* bd = ImGui_Implbgfx_GetBackendData();

    const bgfx::Caps* caps = bgfx::getCaps();
    {
        float ortho[16];
        float x      = draw_data->DisplayPos.x;
        float y      = draw_data->DisplayPos.y;
        float width  = draw_data->DisplaySize.x;
        float height = draw_data->DisplaySize.y;

        bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
        bgfx::setViewTransform(bd->viewId, NULL, ortho);
        bgfx::setViewRect(bd->viewId, 0, 0, uint16_t(width), uint16_t(height));
    }

    const ImVec2 clip_off = draw_data->DisplayPos;
    const ImVec2 clip_scale = draw_data->FramebufferScale;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;
        const ImDrawList* draw_list = draw_data->CmdLists[n];

        uint32_t vtxBufferSize = draw_list->VtxBuffer.size();
        uint32_t idxBufferSize = draw_list->IdxBuffer.size();

        if (!checkAvailTransientBuffers(vtxBufferSize, bd->layout, idxBufferSize))
        {
            // not enough space in transient buffer just quit drawing the rest...
            break;
        }

        bgfx::allocTransientVertexBuffer(&tvb, idxBufferSize, bd->layout);
        bgfx::allocTransientIndexBuffer(&tib, idxBufferSize, sizeof(ImDrawIdx) == 4);

        ImDrawVert* verts = (ImDrawVert*)tvb.data;
        bx::memCopy(verts, draw_list->VtxBuffer.begin(), vtxBufferSize * sizeof(ImDrawVert));
        ImDrawIdx* indices = (ImDrawIdx*)tib.data;
        bx::memCopy(indices, draw_list->IdxBuffer.begin(), idxBufferSize * sizeof(ImDrawIdx));

        bgfx::Encoder* encoder = bgfx::begin();

        for (const ImDrawCmd* cmd = draw_list->CmdBuffer.begin(), *cmdEnd = draw_list->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
        {
            if (cmd->UserCallback)
            {
                cmd->UserCallback(draw_list, cmd);
            }
            else if (0 != cmd->ElemCount)
            {
                uint64_t state = 0
                    | BGFX_STATE_WRITE_RGB
                    | BGFX_STATE_WRITE_A
                    | BGFX_STATE_MSAA
                    ;

                bgfx::TextureHandle th = bd->fontTexture;
                bgfx::ProgramHandle program = bd->IM_Program;

                if (NULL != cmd->TextureId)
                {
                    union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
                    state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
                        ? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                        : BGFX_STATE_NONE
                        ;
                    th = texture.s.handle;
                    if (0 != texture.s.mip)
                    {
                        const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
                        bgfx::setUniform(bd->u_imageLodEnabled, lodEnabled);
                        program = bd->IM_ImageProgram;
                    }
                }
                else
                {
                    state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
                }

                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clipRect;
                clipRect.x = (cmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clipRect.y = (cmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clipRect.z = (cmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clipRect.w = (cmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clipRect.x <  fb_width
                &&  clipRect.y <  fb_height
                &&  clipRect.z >= 0.0f
                &&  clipRect.w >= 0.0f)
                {
                    const uint16_t xx = uint16_t(bx::max(clipRect.x, 0.0f) );
                    const uint16_t yy = uint16_t(bx::max(clipRect.y, 0.0f) );
                    encoder->setScissor(xx, yy
                            , uint16_t(bx::min(clipRect.z, 65535.0f)-xx)
                            , uint16_t(bx::min(clipRect.w, 65535.0f)-yy)
                            );

                    encoder->setState(state);
                    encoder->setTexture(0, bd->s_tex, th);
                    encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset, vtxBufferSize);
                    encoder->setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
                    encoder->submit(bd->viewId, program);
                }
            }
        }

        bgfx::end(encoder);
    }
}

bool ImGui_Implbgfx_InvalidateDeviceObjects() {
    return true;
}