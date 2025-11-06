#include "artnet_poll_reply.h"
#include "esphome/core/log.h"
#include <cstring>

namespace esphome::artnet {

#define ART_NET_ID "Art-Net"
static const char *const TAG = "artnet_poll_reply";

void build_art_poll_reply(uint8_t *poll_reply, const IPAddress &local_ip,
                          uint8_t net, uint8_t subnet,
                          const std::string &short_name,
                          const std::string &long_name, uint16_t poll_counter) {
  // Clear the entire reply buffer
  memset(poll_reply, 0, ART_POLL_REPLY_LENGTH);

  // ID (8 bytes at offset 0-7): "Art-Net\0"
  memcpy(poll_reply, ART_NET_ID, strlen(ART_NET_ID));

  // OpCode (2 bytes at offset 8-9, little-endian)
  uint16_t opcode = ART_POLL_REPLY_OPCODE;
  poll_reply[8] = opcode;
  poll_reply[9] = opcode >> 8;

  // IP Address (4 bytes at offset 10-13)
  poll_reply[10] = local_ip[0];
  poll_reply[11] = local_ip[1];
  poll_reply[12] = local_ip[2];
  poll_reply[13] = local_ip[3];

  // Port (2 bytes at offset 14-15, little-endian)
  poll_reply[14] = ART_PORT & 0xFF;
  poll_reply[15] = (ART_PORT >> 8) & 0xFF;

  // Version (2 bytes at offset 16-17, big-endian)
  const uint16_t version = 14;
  poll_reply[16] = version >> 8;
  poll_reply[17] = version & 0xFF;

  // Net address (1 byte at offset 18)
  poll_reply[18] = net;

  // Subnet address (1 byte at offset 19)
  poll_reply[19] = subnet;

  // OEM Code (2 bytes at offset 20-21, big-endian) - 0xFFFF for developer
  poll_reply[20] = 0xFF;
  poll_reply[21] = 0xFF;

  // UBEA Version (1 byte at offset 22)
  poll_reply[22] = 0;

  // Status1 (1 byte at offset 23)
  // Bit 0-2: Node status
  // Bit 3: Booting
  // Bit 4: RDM capable
  // Bit 5: LLRP capable
  // Bit 6: 0 = DMX out via ArtNet, 1 = DMX out via DMX port
  // Bit 7: 0 = Not indicating shortname/longname, 1 = Indicating
  poll_reply[23] = 0x00; // Normal operation

  // Short name (18 bytes at offset 26-43)
  size_t short_len = short_name.length();
  short_len = (short_len > 17) ? 17 : short_len;
  memcpy(poll_reply + 26, short_name.c_str(), short_len);

  // Long name (64 bytes at offset 44-107)
  size_t long_len = long_name.length();
  long_len = (long_len > 63) ? 63 : long_len;
  memcpy(poll_reply + 44, long_name.c_str(), long_len);

  // NodeReport (64 bytes at offset 108-171)
  // Format: "#xxxx [yyyy] zzzzz..."
  // xxxx = hex status code (0x0001 = RcPowerOk)
  // yyyy = decimal counter (0-9999, rolls over)
  // zzzzz = English text status
  char node_report[64];
  memset(node_report, 0, sizeof(node_report));
  snprintf(node_report, sizeof(node_report),
           "#0001 [%04d] RcPowerOk Power On Tests successful", poll_counter);
  memcpy(poll_reply + 108, node_report, strlen(node_report));

  poll_reply[173] = 0x02; // Port count: 2

  // PortTypes (4 bytes at offset 174-177)
  // Each byte defines the operation and protocol of each channel
  // Bit 7: Set if this channel can output data from the ArtNet Network
  // Bit 6: Set if this channel can input onto the Art-Net Network
  // Bits 5-0: Protocol type (000000=DMX512, 000101=Art-Net, etc.)
  // This device: 2 DMX512 ports capable of input/output
  poll_reply[174] = 0xc0; // Port 0: input + output, DMX512
  poll_reply[175] = 0xc0; // Port 1: input + output, DMX512
}

} // namespace esphome::artnet
