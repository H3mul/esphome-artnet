#include "artnet.h"
#include "artnet_output.h"
#include "artnet_sensor.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/core/log.h"

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

    // Check if it's time to flush outputs
    uint32_t now = millis();
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
      ESP_LOGCONFIG(TAG, "  Universe %d -> DMX component at %p", route.second,
                    route.first);
    }
  }

  if (!dmx_to_artnet_routes_.empty()) {
    ESP_LOGCONFIG(TAG, "DMX to ArtNet routes:");
    for (const auto &route : dmx_to_artnet_routes_) {
      ESP_LOGCONFIG(TAG, "  DMX component at %p -> Universe %d", route.first,
                    route.second);
    }
  }
#endif
}

void ArtNet::send_outputs_data() {
  for (const auto &[universe, outputs] : outputs_per_universe_) {
    uint8_t *dmx_buffer = artnet_->getDmxFrame();
    // Clear DMX buffer
    memset(dmx_buffer, 0, DMX_MAX_CHANNELS);
    for (auto *output : outputs) {
      uint16_t channel = output->get_channel();
      if (channel >= 1 && channel <= DMX_MAX_CHANNELS) {
        dmx_buffer[channel - 1] = output->get_current_value();
      }
    }

    // Calculate the full universe address: net (7 bits) + subnet (4 bits) +
    // universe (4 bits)
    uint16_t full_universe =
        calculate_artnet_universe(this->net_, this->subnet_, universe);
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

void ArtNet::on_artnet_frame(uint16_t universe, uint16_t length,
                             uint8_t sequence, uint8_t *data) {
  ESP_LOGV(TAG, "Received Art-Net frame: universe=%d, length=%d, sequence=%d",
           universe, length, sequence);

  // Update registered sensors
  for (auto *sensor : sensors_) {
    if (sensor->get_universe() == universe && sensor->get_channel() <= length) {
      sensor->update_value(data[sensor->get_channel() - 1]);
    }
  }

  // Route ArtNet data to DMX if configured
  route_artnet_to_dmx(universe, length, sequence, data);
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
    uint16_t full_universe =
        calculate_artnet_universe(this->net_, this->subnet_, universe);
    artnet_->setUniverse(full_universe);
    artnet_->setLength(DMX_MAX_CHANNELS);
    artnet_->write(output_address_);
  }
#endif
}

void ArtNet::route_artnet_to_dmx(uint16_t universe, uint16_t length,
                                 uint8_t sequence, uint8_t *data) {
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
    }
  }
#endif
}

} // namespace esphome::artnet
