#pragma once

#include <d3d11.h>
#include <Windows.h>

class DX11Backend {
public:
    bool init(HWND hwnd);
    void shutdown();
    void resize(UINT width, UINT height);

    void begin_frame(float clear_color[4]);
    void end_frame();

    ID3D11Device*        device()  const { return m_device; }
    ID3D11DeviceContext*  context() const { return m_context; }

private:
    void create_render_target();
    void cleanup_render_target();

    HWND                     m_hwnd        = nullptr;
    IDXGISwapChain*          m_swap_chain  = nullptr;
    ID3D11Device*            m_device      = nullptr;
    ID3D11DeviceContext*     m_context     = nullptr;
    ID3D11RenderTargetView*  m_rtv         = nullptr;
};
