#pragma once

#include "freed_protocol.h"
#include "utils/network_utils.h"
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

class FreeDReceiver {
public:
    FreeDReceiver() = default;
    ~FreeDReceiver();

    bool start(const std::string& bind_ip, uint16_t port);
    void stop();
    bool is_running() const { return m_running.load(); }

    // Returns true if a new packet was available since the last poll
    bool poll_latest(FreeDPacket& out);

    uint64_t packets_received() const { return m_packets_received.load(); }
    uint64_t packets_invalid() const { return m_packets_invalid.load(); }
    std::string last_error() const;

private:
    void receive_loop();

    std::thread          m_thread;
    std::atomic<bool>    m_running{false};
    std::atomic<bool>    m_should_stop{false};
    SOCKET               m_socket = INVALID_SOCKET;

    std::mutex           m_mutex;
    FreeDPacket          m_latest_packet{};
    bool                 m_has_new_packet = false;

    std::atomic<uint64_t> m_packets_received{0};
    std::atomic<uint64_t> m_packets_invalid{0};

    mutable std::mutex   m_error_mutex;
    std::string          m_last_error;
};
