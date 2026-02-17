#include "app.h"
#include <cstdio>
#include <cstring>

bool App::init(HWND hwnd) {
    if (!m_dx11.init(hwnd)) return false;
    m_last_pps_time = std::chrono::steady_clock::now();
    snprintf(m_state.status_msg, sizeof(m_state.status_msg), "Ready");
    return true;
}

void App::shutdown() {
    m_receiver.stop();
    m_osc_sender.disconnect();
    m_dx11.shutdown();
}

void App::tick() {
    // Handle GUI requests (start/stop listening, connect/disconnect OSC)
    handle_requests();

    // Poll latest FreeD data
    FreeDPacket pkt;
    if (m_receiver.poll_latest(pkt)) {
        m_state.current_packet = pkt;
        m_state.has_data = true;

        // Forward to OSC if enabled
        if (m_osc_sender.is_connected() && m_osc_config.enabled) {
            m_osc_config.mode = static_cast<OscSendMode>(m_state.osc_mode);
            m_osc_config.prefix = m_state.osc_prefix;
            m_osc_sender.send(pkt, m_osc_config);
        }
    }

    // Update stats
    m_state.packets_received = m_receiver.packets_received();
    m_state.packets_invalid = m_receiver.packets_invalid();
    m_state.freed_listening = m_receiver.is_running();
    m_state.osc_connected = m_osc_sender.is_connected();
    update_stats();

    // Update status message
    if (m_state.freed_listening && m_state.osc_connected) {
        snprintf(m_state.status_msg, sizeof(m_state.status_msg),
                 "Listening on %s:%d | OSC -> %s:%d",
                 m_state.freed_bind_ip, m_state.freed_port,
                 m_state.osc_dest_ip, m_state.osc_port);
    } else if (m_state.freed_listening) {
        snprintf(m_state.status_msg, sizeof(m_state.status_msg),
                 "Listening on %s:%d", m_state.freed_bind_ip, m_state.freed_port);
    } else {
        snprintf(m_state.status_msg, sizeof(m_state.status_msg), "Ready");
    }

    // Check for receiver errors
    std::string err = m_receiver.last_error();
    if (!err.empty() && !m_receiver.is_running() && m_state.freed_listening) {
        snprintf(m_state.status_msg, sizeof(m_state.status_msg),
                 "Error: %s", err.c_str());
    }

    // Render
    float clear_color[4] = { 0.06f, 0.06f, 0.06f, 1.00f };
    m_dx11.begin_frame(clear_color);
    m_gui.render(m_state);
    m_dx11.end_frame();
}

void App::on_resize(UINT width, UINT height) {
    m_dx11.resize(width, height);
}

void App::handle_requests() {
    // FreeD start/stop
    if (m_state.freed_start_req) {
        m_state.freed_start_req = false;
        m_receiver.start(m_state.freed_bind_ip,
                         static_cast<uint16_t>(m_state.freed_port));
    }
    if (m_state.freed_stop_req) {
        m_state.freed_stop_req = false;
        m_receiver.stop();
    }

    // OSC connect/disconnect
    if (m_state.osc_connect_req) {
        m_state.osc_connect_req = false;
        if (m_osc_sender.connect(m_state.osc_dest_ip,
                                  static_cast<uint16_t>(m_state.osc_port))) {
            m_osc_config.enabled = true;
            m_osc_config.dest_ip = m_state.osc_dest_ip;
            m_osc_config.dest_port = static_cast<uint16_t>(m_state.osc_port);
        }
    }
    if (m_state.osc_disconnect_req) {
        m_state.osc_disconnect_req = false;
        m_osc_config.enabled = false;
        m_osc_sender.disconnect();
    }
}

void App::update_stats() {
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - m_last_pps_time).count();
    if (elapsed >= 1.0f) {
        uint64_t current = m_state.packets_received;
        m_state.packets_per_sec = static_cast<float>(current - m_prev_packet_count) / elapsed;
        m_prev_packet_count = current;
        m_last_pps_time = now;
    }
}
