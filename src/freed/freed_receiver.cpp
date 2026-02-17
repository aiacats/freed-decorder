#include "freed_receiver.h"

FreeDReceiver::~FreeDReceiver() {
    stop();
}

bool FreeDReceiver::start(const std::string& bind_ip, uint16_t port) {
    if (m_running.load()) return false;

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET) {
        std::lock_guard<std::mutex> lock(m_error_mutex);
        m_last_error = "Failed to create socket: " + std::to_string(WSAGetLastError());
        return false;
    }

    // Set receive timeout (100ms) so we can check the stop flag
    DWORD timeout_ms = 100;
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms));

    // Allow address reuse
    int reuse = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, bind_ip.c_str(), &addr.sin_addr) != 1) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        std::lock_guard<std::mutex> lock(m_error_mutex);
        m_last_error = "Invalid IP address: " + bind_ip;
        return false;
    }

    if (bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::lock_guard<std::mutex> lock(m_error_mutex);
        m_last_error = "Bind failed: " + std::to_string(WSAGetLastError());
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    m_should_stop.store(false);
    m_running.store(true);
    m_packets_received.store(0);
    m_packets_invalid.store(0);
    {
        std::lock_guard<std::mutex> lock(m_error_mutex);
        m_last_error.clear();
    }

    m_thread = std::thread(&FreeDReceiver::receive_loop, this);
    return true;
}

void FreeDReceiver::stop() {
    if (!m_running.load()) return;
    m_should_stop.store(true);
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    m_running.store(false);
}

bool FreeDReceiver::poll_latest(FreeDPacket& out) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_has_new_packet) return false;
    out = m_latest_packet;
    m_has_new_packet = false;
    return true;
}

std::string FreeDReceiver::last_error() const {
    std::lock_guard<std::mutex> lock(m_error_mutex);
    return m_last_error;
}

void FreeDReceiver::receive_loop() {
    uint8_t buffer[256];
    sockaddr_in sender_addr{};
    int sender_len = sizeof(sender_addr);

    while (!m_should_stop.load()) {
        int bytes = recvfrom(m_socket, reinterpret_cast<char*>(buffer),
                             sizeof(buffer), 0,
                             reinterpret_cast<sockaddr*>(&sender_addr),
                             &sender_len);
        if (bytes == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) continue;
            std::lock_guard<std::mutex> lock(m_error_mutex);
            m_last_error = "recvfrom error: " + std::to_string(err);
            break;
        }

        if (bytes >= static_cast<int>(FREED_PACKET_SIZE)) {
            auto decoded = decode_freed_packet(buffer, static_cast<size_t>(bytes));
            if (decoded) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_latest_packet = *decoded;
                m_has_new_packet = true;
                m_packets_received.fetch_add(1);
            } else {
                m_packets_invalid.fetch_add(1);
            }
        }
    }
    m_running.store(false);
}
