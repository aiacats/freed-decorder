#include "osc_sender.h"
#include <oscpp/client.hpp>

OscSender::~OscSender() {
    disconnect();
}

bool OscSender::connect(const std::string& dest_ip, uint16_t dest_port) {
    disconnect();

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET) return false;

    m_dest_addr = {};
    m_dest_addr.sin_family = AF_INET;
    m_dest_addr.sin_port = htons(dest_port);
    if (inet_pton(AF_INET, dest_ip.c_str(), &m_dest_addr.sin_addr) != 1) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    m_connected = true;
    return true;
}

void OscSender::disconnect() {
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    m_connected = false;
}

void OscSender::send(const FreeDPacket& packet, const OscConfig& config) {
    if (!m_connected || !config.enabled) return;

    if (config.mode == OscSendMode::Individual) {
        send_individual(packet, config.prefix);
    } else {
        send_bundled(packet, config.prefix);
    }
}

void OscSender::send_individual(const FreeDPacket& packet, const std::string& prefix) {
    send_osc_float(prefix + "/pos/x",    packet.pos_x);
    send_osc_float(prefix + "/pos/y",    packet.pos_y);
    send_osc_float(prefix + "/pos/z",    packet.pos_z);
    send_osc_float(prefix + "/rot/pan",  packet.yaw);
    send_osc_float(prefix + "/rot/tilt", packet.pitch);
    send_osc_float(prefix + "/rot/roll", packet.roll);
    send_osc_float(prefix + "/zoom",     static_cast<float>(packet.zoom));
    send_osc_float(prefix + "/focus",    static_cast<float>(packet.focus));
}

void OscSender::send_bundled(const FreeDPacket& packet, const std::string& prefix) {
    std::string addr = prefix + "/transform";
    OSCPP::Client::Packet p(m_buffer, BUFFER_SIZE);
    p.openMessage(addr.c_str(), 8)
        .float32(packet.pos_x)
        .float32(packet.pos_y)
        .float32(packet.pos_z)
        .float32(packet.yaw)
        .float32(packet.pitch)
        .float32(packet.roll)
        .float32(static_cast<float>(packet.zoom))
        .float32(static_cast<float>(packet.focus))
    .closeMessage();
    send_buffer(p.data(), p.size());
}

void OscSender::send_osc_float(const std::string& address, float value) {
    OSCPP::Client::Packet p(m_buffer, BUFFER_SIZE);
    p.openMessage(address.c_str(), 1)
        .float32(value)
    .closeMessage();
    send_buffer(p.data(), p.size());
}

void OscSender::send_buffer(const void* data, size_t size) {
    sendto(m_socket, reinterpret_cast<const char*>(data),
           static_cast<int>(size), 0,
           reinterpret_cast<const sockaddr*>(&m_dest_addr),
           sizeof(m_dest_addr));
}
