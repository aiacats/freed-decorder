#pragma once

#include "freed/freed_receiver.h"
#include "osc/osc_sender.h"
#include "gui/gui_manager.h"
#include "gui/dx11_backend.h"
#include <chrono>

class App {
public:
    bool init(HWND hwnd);
    void shutdown();
    void tick();
    void on_resize(UINT width, UINT height);

    DX11Backend& dx11() { return m_dx11; }

private:
    void handle_requests();
    void update_stats();

    DX11Backend     m_dx11;
    GuiManager      m_gui;
    AppState        m_state{};
    FreeDReceiver   m_receiver;
    OscSender       m_osc_sender;
    OscConfig       m_osc_config;

    // For packets-per-second calculation
    uint64_t m_prev_packet_count = 0;
    std::chrono::steady_clock::time_point m_last_pps_time;
};
