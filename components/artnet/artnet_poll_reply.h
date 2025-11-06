#pragma once

#include <WiFi.h>
#include <cstdint>
#include <string>

namespace esphome::artnet {

// Art-Net Poll Reply constants
static const uint16_t ART_POLL_REPLY_OPCODE = 0x2100;
static const uint16_t ART_POLL_REPLY_LENGTH = 207;
static const uint16_t ART_PORT = 6454;

/**
 * Builds an ArtPollReply frame with the provided device information.
 *
 * @param poll_reply Output buffer (must be at least ART_POLL_REPLY_LENGTH
 * bytes)
 * @param local_ip The device's local IP address
 * @param net The Art-Net net value (0-127)
 * @param subnet The Art-Net subnet value (0-15)
 * @param short_name Short device name (max 17 chars)
 * @param long_name Long device name (max 63 chars)
 * @param poll_counter Poll response counter (0-9999)
 */
void build_art_poll_reply(uint8_t *poll_reply, const IPAddress &local_ip,
                          uint8_t net, uint8_t subnet,
                          const std::string &short_name,
                          const std::string &long_name, uint16_t poll_counter);

} // namespace esphome::artnet
