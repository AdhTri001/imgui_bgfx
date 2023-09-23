#include "bgfx/bgfx.h"
#include "imgui.h"

#include "imgui_impl_bgfx.h"

struct ImGui_Implbgfx_Data
{
    uint8_t viewId;
    bgfx::ProgramHandle ShaderHandle;

    ImGui_Implbgfx_Data() { memset((void*)this, 0, sizeof(*this)); }
};

static ImGui_Implbgfx_Data* ImGui_Implbgfx_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_Implbgfx_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

bool ImGui_Implbgfx_Init(int view) {
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    ImGui_Implbgfx_Data* bd = IM_NEW(ImGui_Implbgfx_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_bgfx";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    bd->viewId = (uint8_t)(view & 0xff);
}

void ImGui_Implbgfx_NewFrame() {
    ImGui_Implbgfx_Data* bd = ImGui_Implbgfx_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplOpenGL3_Init()?");

    if(!bd->ShaderHandle)
        ImGui_Implbgfx_CreateDeviceObjects();
}

bool ImGui_Implbgfx_CreateDeviceObjects() {
}