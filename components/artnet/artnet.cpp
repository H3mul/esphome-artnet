#include "artnet.h"
#include "artnet_sensor.h"
#include "esphome/core/log.h"

namespace esphome::artnet {

static const char *const TAG = "artnet";

// Static member definitions
ArtnetWifi *ArtNet::artnet_ = nullptr;
ArtNet *ArtNet::instance_ = nullptr;
std::vector<ArtNetSensor *> ArtNet::sensors_;

void ArtNet::register_sensor(ArtNetSensor *sensor) {
  sensors_.push_back(sensor);
}

void ArtNet::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ArtNet...");

  // Store instance for callback
  instance_ = this;

  // Initialize the ArtNet library
  artnet_ = new ArtnetWifi();

  // Register the Art-Net DMX callback
  artnet_->setArtDmxCallback(artnet_callback);

  // Start listening for ArtNet packets
  artnet_->begin();

  ESP_LOGCONFIG(TAG, "ArtNet setup complete");
}

void ArtNet::loop() {
  // Process ArtNet packets
  this->artnet_->read();
}

void ArtNet::dump_config() {
  ESP_LOGCONFIG(TAG, "ArtNet:");
  ESP_LOGCONFIG(TAG, "  Listening for ArtNet packets");
}

void ArtNet::on_artnet_frame(uint16_t universe, uint16_t length,
                             uint8_t sequence, uint8_t *data) {
  ESP_LOGD(TAG, "Received Art-Net frame: universe=%d, length=%d, sequence=%d",
           universe, length, sequence);

  // Update registered sensors
  for (auto *sensor : sensors_) {
    if (sensor->get_universe() == universe && sensor->get_channel() <= length) {
      sensor->update_value(data[sensor->get_channel() - 1]);
    }
  }
}

void ArtNet::artnet_callback(uint16_t universe, uint16_t length,
                             uint8_t sequence, uint8_t *data) {
  if (instance_ != nullptr) {
    instance_->on_artnet_frame(universe, length, sequence, data);
  }
}

} // namespace esphome::artnet
