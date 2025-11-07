#include "artnet.h"
#include "artnet_output.h"
#include "artnet_poll_reply.h"
#include "artnet_sensor.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/core/log.h"
#include <cstdint>
#include <cstring>

#ifdef USE_DMX_COMPONENT
#include "esphome/components/dmx/dmx.h"
#endif

#define DMX_MAX_CHANNELS 512
namespace esphome::artnet {

static const char *const TAG = "artnet";

// Static member definitions
ArtnetWifi *ArtNet::artnet_ = nullptr;
ArtNet *ArtNet::instance_ = nullptr;
std::vector<ArtNetSensor *> ArtNet::sensors_;
std::map<uint16_t, std::vector<ArtNetOutput *>> ArtNet::outputs_per_universe_;

// Helper function to calculate full Art-Net universe address
// Combines net (7 bits) + subnet (4 bits) + universe (4 bits) into a 15-bit
// address
static uint16_t calculate_artnet_universe(uint8_t net, uint8_t subnet,
                                          uint16_t universe) {
  return (net << 8) | (subnet << 4) | universe;
}

// Helper function to parse full Art-Net universe address
// Extracts net (7 bits), subnet (4 bits), and universe (4 bits) from a 15-bit
// address
static void parse_artnet_universe(uint16_t full_universe, uint8_t &net,
                                  uint8_t &subnet, uint8_t &universe) {
  net = (full_universe >> 8) & 0x7F;    // 7 bits (bits 14-8)
  subnet = (full_universe >> 4) & 0x0F; // 4 bits (bits 7-4)
  universe = full_universe & 0x0F;      // 4 bits (bits 3-0)
}

void ArtNet::register_sensor(ArtNetSensor *sensor) {
  sensors_.push_back(sensor);
}

void ArtNet::register_output(ArtNetOutput *output) {
  outputs_per_universe_[output->get_universe()].push_back(output);
}

void ArtNet::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ArtNet...");

  instance_ = this;

  artnet_ = new ArtnetWifi();
  artnet_->setArtDmxCallback(artnet_callback);
  artnet_->begin();
}

void ArtNet::loop() {
  if (wifi::global_wifi_component->is_connected()) {
    uint32_t opcode = artnet_->read();
    if (opcode != 0) {
      ESP_LOGV(TAG, "Received Art-Net frame with opcode: %u", opcode);
    }

    // Handle ArtPoll by scheduling a reply
    if (opcode == ART_POLL) {
      this->poll_reply_sender_ip_ = artnet_->getSenderIp();
      this->last_poll_reply_time_ = millis();
      // Generate random delay between 0 and 1000ms per Art-Net spec
      this->poll_reply_delay_ms_ = random(0, POLL_REPLY_MAX_DELAY_MS + 1);
    }

    // Check if it's time to send a pending ArtPollReply
    uint32_t now = millis();
    if (this->poll_reply_sender_ip_ != IPAddress(0, 0, 0, 0) &&
        (now - this->last_poll_reply_time_) >= this->poll_reply_delay_ms_) {
      this->send_poll_reply();
      this->poll_reply_sender_ip_ = IPAddress(0, 0, 0, 0); // Clear
    }

    // Check if it's time to flush outputs
    if (now - this->last_flush_time_ >= this->flush_period_ms_) {
      this->last_flush_time_ = now;
      this->send_outputs_data();

#ifdef USE_DMX_COMPONENT
      this->route_dmx_to_artnet();
#endif
    }
  }
}

void ArtNet::dump_config() {
  ESP_LOGCONFIG(TAG, "ArtNet:");
  ESP_LOGCONFIG(TAG, "  Listening for ArtNet packets");
  ESP_LOGCONFIG(TAG, "  Output Address: %s",
                this->output_address_.toString().c_str());

#ifdef USE_DMX_COMPONENT
  // Log routing configuration
  if (!artnet_to_dmx_routes_.empty()) {
    ESP_LOGCONFIG(TAG, "ArtNet to DMX routes:");
    for (const auto &route : artnet_to_dmx_routes_) {
      ESP_LOGCONFIG(TAG, "  DMX component at %s -> Universe %d",
                    route.first->get_name().c_str(), route.second);
    }
  }

  if (!dmx_to_artnet_routes_.empty()) {
    ESP_LOGCONFIG(TAG, "DMX to ArtNet routes:");
    for (const auto &route : dmx_to_artnet_routes_) {
      ESP_LOGCONFIG(TAG, "  DMX component at %s -> Universe %d",
                    route.first->get_name().c_str(), route.second);
    }
  }
#endif
}

void ArtNet::send_outputs_data() {
  for (const auto &[universe, outputs] : outputs_per_universe_) {
    bool has_changes = false;
    for (auto *output : outputs) {
      uint16_t channel = output->get_channel();
      if (channel >= 1 && channel <= DMX_MAX_CHANNELS) {
        has_changes |= output->has_unflushed_changes();
      }
    }

    // quit early, if we don't have any changes to send for this universe
    if (!has_changes && !this->continuous_output_) {
      continue;
    }

    uint8_t *buffer = artnet_->getDmxFrame();
    // Clear DMX buffer
    memset(buffer, 0, DMX_MAX_CHANNELS);

    for (auto *output : outputs) {
      uint16_t channel = output->get_channel();
      if (channel >= 1 && channel <= DMX_MAX_CHANNELS) {
        buffer[channel - 1] = output->get_current_value();
        output->set_changes_flushed();
      }
    }

    uint16_t full_universe = calculate_artnet_universe(
        this->output_net_, this->output_subnet_, universe);
    artnet_->setUniverse(full_universe);
    artnet_->setLength(DMX_MAX_CHANNELS);
    artnet_->write(output_address_);
  }
}

void ArtNet::artnet_callback(uint16_t universe, uint16_t length,
                             uint8_t sequence, uint8_t *data) {
  if (instance_ != nullptr) {
    instance_->on_artnet_frame(universe, length, sequence, data);
  }
}

void ArtNet::on_artnet_frame(uint16_t full_universe, uint16_t length,
                             uint8_t sequence, uint8_t *data) {
  // Parse the full universe address into net, subnet, and universe components
  uint8_t net;
  uint8_t subnet;
  uint8_t universe;
  parse_artnet_universe(full_universe, net, subnet, universe);

  // Filter frames that don't match our configured net and subnet
  if (net != this->net_ || subnet != this->subnet_) {
    return; // Ignore frames from other nets/subnets
  }

  ESP_LOGV(TAG,
           "Received Art-Net frame: net=%d, subnet=%d, universe=%d, "
           "length=%d, sequence=%d",
           net, subnet, universe, length, sequence);

  // Update registered sensors with the actual universe index
  for (auto *sensor : sensors_) {
    if (sensor->get_universe() == universe && sensor->get_channel() <= length) {
      sensor->update_value(data[sensor->get_channel() - 1]);
    }
  }

  // Route ArtNet data to DMX if configured
  route_artnet_to_dmx(universe, data, length);
}
void ArtNet::route_dmx_to_artnet() {
#ifdef USE_DMX_COMPONENT
  // Iterate over all DMX to ArtNet routes
  for (const auto &route : dmx_to_artnet_routes_) {
    esphome::dmx::DMXComponent *dmx_component = route.first;
    uint16_t universe = route.second;

    if (dmx_component == nullptr) {
      ESP_LOGW(TAG, "DMX component pointer is null for routing");
      continue;
    }

    // Prepare DMX buffer
    uint8_t *dmx_data = artnet_->getDmxFrame();

    // Read the full DMX universe from the DMX component
    dmx_component->read_universe(dmx_data, DMX_MAX_CHANNELS);

    // Send the DMX data as an Art-Net frame
    uint16_t full_universe = calculate_artnet_universe(
        this->output_net_, this->output_subnet_, universe);
    artnet_->setUniverse(full_universe);
    artnet_->setLength(DMX_MAX_CHANNELS);
    artnet_->write(output_address_);
    ESP_LOGVV(TAG, "Sent frame from DMX to Art-Net for universe %d", universe);
  }
#endif
}

void ArtNet::route_artnet_to_dmx(uint8_t universe, uint8_t *data,
                                 uint16_t length) {
  // Check if this universe should be routed to DMX
#ifdef USE_DMX_COMPONENT
  for (const auto &route : artnet_to_dmx_routes_) {
    if (route.second == universe) {
      esphome::dmx::DMXComponent *dmx_component = route.first;

      if (dmx_component == nullptr) {
        ESP_LOGW(TAG, "DMX component pointer is null for routing");
        continue;
      }

      // Write the full DMX universe to the DMX component
      dmx_component->send_universe(data, length);
      ESP_LOGVV(TAG, "Sent frame from Art-Net to DMX for universe %d",
                universe);
    }
  }
#endif
}

void ArtNet::send_poll_reply() {
  uint8_t poll_reply[ART_POLL_REPLY_LENGTH];

  // Increment and roll over the poll response counter
  this->poll_response_counter_ = (this->poll_response_counter_ + 1) % 10000;

  // Build the poll reply frame
  build_art_poll_reply(poll_reply, WiFi.localIP(), this->net_, this->subnet_,
                       this->name_short_, this->name_long_,
                       this->poll_response_counter_);

  // Send the reply via UDP to the sender's IP
  WiFiUDP Udp;
  Udp.beginPacket(this->poll_reply_sender_ip_, ART_NET_PORT);
  Udp.write(poll_reply, sizeof(poll_reply));
  Udp.endPacket();

  ESP_LOGD(TAG, "Sent ArtPollReply to %s",
           this->poll_reply_sender_ip_.toString().c_str());
}

} // namespace esphome::artnet
