#pragma once

#include "freed/freed_protocol.h"
#include "utils/network_utils.h"
#include <string>
#include <cstdint>

enum class OscSendMode {
    Individual = 0,
    Bundled = 1
};

struct OscConfig {
    std::string dest_ip   = "127.0.0.1";
    uint16_t    dest_port = 9000;
    OscSendMode mode      = OscSendMode::Individual;
    std::string prefix    = "/freed";
    bool        enabled   = false;
};

class OscSender {
public:
    OscSender() = default;
    ~OscSender();

    bool connect(const std::string& dest_ip, uint16_t dest_port);
    void disconnect();
    bool is_connected() const { return m_connected; }

    void send(const FreeDPacket& packet, const OscConfig& config);

private:
    void send_individual(const FreeDPacket& packet, const std::string& prefix);
    void send_bundled(const FreeDPacket& packet, const std::string& prefix);
    void send_osc_float(const std::string& address, float value);
    void send_buffer(const void* data, size_t size);

    SOCKET       m_socket = INVALID_SOCKET;
    sockaddr_in  m_dest_addr{};
    bool         m_connected = false;

    static constexpr size_t BUFFER_SIZE = 1024;
    char m_buffer[BUFFER_SIZE]{};
};
