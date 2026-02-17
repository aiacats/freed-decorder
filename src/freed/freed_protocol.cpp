#include "freed_protocol.h"

int32_t decode_signed_24bit(const uint8_t* b) {
    int32_t value = (static_cast<int32_t>(b[0]) << 16)
                  | (static_cast<int32_t>(b[1]) << 8)
                  |  static_cast<int32_t>(b[2]);
    // Sign-extend from 24-bit to 32-bit
    if (value & 0x800000) {
        value |= static_cast<int32_t>(0xFF000000);
    }
    return value;
}

bool verify_freed_checksum(const uint8_t* data, size_t len) {
    if (len < FREED_PACKET_SIZE) return false;
    uint8_t sum = 0;
    for (size_t i = 0; i < FREED_PACKET_SIZE - 1; ++i) {
        sum += data[i];
    }
    uint8_t expected = (0x40 - sum) & 0xFF;
    return data[FREED_PACKET_SIZE - 1] == expected;
}

std::optional<FreeDPacket> decode_freed_packet(const uint8_t* data, size_t len) {
    if (len < FREED_PACKET_SIZE) return std::nullopt;
    if (data[0] != FREED_MSG_TYPE_D1) return std::nullopt;
    if (!verify_freed_checksum(data, len)) return std::nullopt;

    FreeDPacket pkt;
    pkt.camera_id = data[1];
    pkt.pitch = decode_signed_24bit(&data[2])  / 32768.0f;
    pkt.yaw   = decode_signed_24bit(&data[5])  / 32768.0f;
    pkt.roll  = decode_signed_24bit(&data[8])  / 32768.0f;
    pkt.pos_z = decode_signed_24bit(&data[11]) / 64.0f;
    pkt.pos_y = decode_signed_24bit(&data[14]) / 64.0f;
    pkt.pos_x = decode_signed_24bit(&data[17]) / 64.0f;
    pkt.zoom  = decode_signed_24bit(&data[20]);
    pkt.focus = decode_signed_24bit(&data[23]);
    return pkt;
}
