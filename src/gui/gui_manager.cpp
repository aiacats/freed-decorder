#include "gui_manager.h"
#include <imgui.h>
#include <cstdio>

void GuiManager::render(AppState& state) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::Begin("FreeD Decoder", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Left column
    ImGui::BeginChild("LeftColumn", ImVec2(320, 0), true);
    render_freed_panel(state);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    render_osc_panel(state);
    ImGui::EndChild();

    ImGui::SameLine();

    // Right column: live data
    ImGui::BeginChild("RightColumn", ImVec2(0, 0), true);
    render_data_panel(state);
    ImGui::EndChild();

    ImGui::End();
}

void GuiManager::render_freed_panel(AppState& state) {
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "FreeD Input");
    ImGui::Separator();
    ImGui::Spacing();

    bool is_listening = state.freed_listening;

    if (is_listening) ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##BindIP", state.freed_bind_ip, sizeof(state.freed_bind_ip));
    ImGui::SameLine(0, 0); // for label alignment trick
    ImGui::Text(" Bind IP");

    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("Port##freed", &state.freed_port);
    if (state.freed_port < 1) state.freed_port = 1;
    if (state.freed_port > 65535) state.freed_port = 65535;
    if (is_listening) ImGui::EndDisabled();

    ImGui::Spacing();

    if (!is_listening) {
        if (ImGui::Button("Start Listening", ImVec2(-1, 30))) {
            state.freed_start_req = true;
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Stop Listening", ImVec2(-1, 30))) {
            state.freed_stop_req = true;
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Status
    if (is_listening) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: Listening");
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Status: Stopped");
    }

    ImGui::Text("Packets: %llu", state.packets_received);
    ImGui::Text("Errors:  %llu", state.packets_invalid);
    ImGui::Text("Rate:    %.1f pkt/s", state.packets_per_sec);
}

void GuiManager::render_osc_panel(AppState& state) {
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "OSC Output");
    ImGui::Separator();
    ImGui::Spacing();

    bool is_connected = state.osc_connected;

    if (is_connected) ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##OscIP", state.osc_dest_ip, sizeof(state.osc_dest_ip));
    ImGui::SameLine(0, 0);
    ImGui::Text(" Dest IP");

    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("Port##osc", &state.osc_port);
    if (state.osc_port < 1) state.osc_port = 1;
    if (state.osc_port > 65535) state.osc_port = 65535;
    if (is_connected) ImGui::EndDisabled();

    ImGui::Spacing();

    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##OscPrefix", state.osc_prefix, sizeof(state.osc_prefix));
    ImGui::SameLine(0, 0);
    ImGui::Text(" Prefix");

    ImGui::Spacing();

    ImGui::Text("Send Mode:");
    ImGui::RadioButton("Individual", &state.osc_mode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Bundled", &state.osc_mode, 1);

    ImGui::Spacing();

    if (!is_connected) {
        if (ImGui::Button("Enable Sending", ImVec2(-1, 30))) {
            state.osc_connect_req = true;
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Disable Sending", ImVec2(-1, 30))) {
            state.osc_disconnect_req = true;
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();
    if (is_connected) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: Sending");
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Status: Disabled");
    }
}

void GuiManager::render_data_panel(AppState& state) {
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Live Data");
    ImGui::Separator();
    ImGui::Spacing();

    const auto& pkt = state.current_packet;

    if (!state.has_data) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No data received yet...");
        return;
    }

    ImGui::Text("Camera ID: %d", pkt.camera_id);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Position and Rotation side by side
    if (ImGui::BeginTable("DataTable", 2, ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Rotation", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("X: %10.3f", pkt.pos_x);
        ImGui::Text("Y: %10.3f", pkt.pos_y);
        ImGui::Text("Z: %10.3f", pkt.pos_z);

        ImGui::TableNextColumn();
        ImGui::Text("Pan:  %10.3f deg", pkt.yaw);
        ImGui::Text("Tilt: %10.3f deg", pkt.pitch);
        ImGui::Text("Roll: %10.3f deg", pkt.roll);

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Lens");
    ImGui::Spacing();

    if (ImGui::BeginTable("LensTable", 2, ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Zoom", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Focus", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%d", pkt.zoom);

        ImGui::TableNextColumn();
        ImGui::Text("%d", pkt.focus);

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Status bar at bottom
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", state.status_msg);
}
