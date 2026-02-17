#pragma once

#include <cstdint>
#include <cstddef>
#include <optional>

static constexpr size_t FREED_PACKET_SIZE = 29;
static constexpr uint8_t FREED_MSG_TYPE_D1 = 0xD1;

struct FreeDPacket {
    uint8_t camera_id = 0;
    float   pitch     = 0.0f;  // rotation X (degrees)
    float   yaw       = 0.0f;  // rotation Y (degrees)
    float   roll      = 0.0f;  // rotation Z (degrees)
    float   pos_x     = 0.0f;  // position X
    float   pos_y     = 0.0f;  // position Y
    float   pos_z     = 0.0f;  // position Z
    int32_t zoom      = 0;     // raw 24-bit signed
    int32_t focus     = 0;     // raw 24-bit signed
};

int32_t decode_signed_24bit(const uint8_t* bytes);
bool verify_freed_checksum(const uint8_t* data, size_t len);
std::optional<FreeDPacket> decode_freed_packet(const uint8_t* data, size_t len);
