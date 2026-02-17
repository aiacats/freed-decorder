#include "dx11_backend.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

bool DX11Backend::init(HWND hwnd) {
    m_hwnd = hwnd;

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT create_flags = 0;
#ifdef _DEBUG
    create_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL feature_level;
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_flags,
        levels, 2, D3D11_SDK_VERSION,
        &sd, &m_swap_chain, &m_device, &feature_level, &m_context);

    if (FAILED(hr)) return false;

    create_render_target();

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);

    return true;
}

void DX11Backend::shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanup_render_target();
    if (m_swap_chain) { m_swap_chain->Release(); m_swap_chain = nullptr; }
    if (m_context)    { m_context->Release();    m_context = nullptr; }
    if (m_device)     { m_device->Release();     m_device = nullptr; }
}

void DX11Backend::resize(UINT width, UINT height) {
    if (width == 0 || height == 0) return;
    cleanup_render_target();
    m_swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    create_render_target();
}

void DX11Backend::begin_frame(float clear_color[4]) {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    m_context->OMSetRenderTargets(1, &m_rtv, nullptr);
    m_context->ClearRenderTargetView(m_rtv, clear_color);
}

void DX11Backend::end_frame() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    m_swap_chain->Present(1, 0);  // vsync on
}

void DX11Backend::create_render_target() {
    ID3D11Texture2D* back_buffer = nullptr;
    m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (back_buffer) {
        m_device->CreateRenderTargetView(back_buffer, nullptr, &m_rtv);
        back_buffer->Release();
    }
}

void DX11Backend::cleanup_render_target() {
    if (m_rtv) { m_rtv->Release(); m_rtv = nullptr; }
}
