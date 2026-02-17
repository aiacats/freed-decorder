#pragma once

#include "freed/freed_protocol.h"
#include "osc/osc_sender.h"
#include <cstdint>

struct AppState {
    // FreeD receiver config
    char     freed_bind_ip[64]   = "0.0.0.0";
    int      freed_port          = 40000;
    bool     freed_listening     = false;
    bool     freed_start_req     = false;  // GUI request to start
    bool     freed_stop_req      = false;  // GUI request to stop

    // OSC sender config
    char     osc_dest_ip[64]     = "127.0.0.1";
    int      osc_port            = 9000;
    bool     osc_enabled         = false;
    bool     osc_connect_req     = false;
    bool     osc_disconnect_req  = false;
    int      osc_mode            = 0;     // 0=Individual, 1=Bundled
    char     osc_prefix[128]     = "/freed";

    // Live data
    FreeDPacket current_packet{};
    bool        has_data         = false;

    // Status
    uint64_t    packets_received = 0;
    uint64_t    packets_invalid  = 0;
    float       packets_per_sec  = 0.0f;
    bool        osc_connected    = false;
    char        status_msg[256]  = "Ready";
};

class GuiManager {
public:
    void render(AppState& state);

private:
    void render_freed_panel(AppState& state);
    void render_osc_panel(AppState& state);
    void render_data_panel(AppState& state);
};
