#include "artnet_receiver.h"
#include "esphome/core/log.h"

namespace esphome::artnet_receiver {

static const char *const TAG = "artnet_receiver";

// Static instance pointer for the callback
ArtNetReceiver *ArtNetReceiver::instance_ = nullptr;

void ArtNetReceiver::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ArtNet Receiver...");

  // Store the instance for the static callback
  instance_ = this;

  // Initialize the ArtNet library
  this->artnet_ = new ArtnetWifi();

  // Set up the callback for DMX data
  this->artnet_->setArtDmxCallback(ArtNetReceiver::on_dmx_frame_callback);

  // Start listening for ArtNet packets
  this->artnet_->begin();

  ESP_LOGCONFIG(TAG, "ArtNet Receiver setup complete. Listening on universe %d",
                this->artnet_universe_);
}

void ArtNetReceiver::loop() {
  // Process ArtNet packets
  this->artnet_->read();
}

void ArtNetReceiver::dump_config() {
  ESP_LOGCONFIG(TAG, "ArtNet Receiver:");
  ESP_LOGCONFIG(TAG, "  Universe: %d", this->artnet_universe_);
  ESP_LOGCONFIG(TAG, "  Number of channels: %d", this->channels_.size());

  for (auto *channel : this->channels_) {
    ESP_LOGCONFIG(TAG, "    Channel %d: %s", channel->get_channel_number(),
                  channel->get_name().c_str());
  }
}

void ArtNetReceiver::on_dmx_frame(uint16_t universe, uint16_t length,
                                  uint8_t sequence, uint8_t *data) {
  // Check if this is the universe we're interested in
  if (universe != this->artnet_universe_) {
    return;
  }

  ESP_LOGV(TAG, "Received DMX frame for universe %d, length %d, sequence %d",
           universe, length, sequence);

  // Update all registered channels with new data
  for (auto *channel : this->channels_) {
    uint16_t channel_number = channel->get_channel_number();

    // DMX channels are 1-based, but array is 0-based
    if (channel_number >= 1 && channel_number <= length) {
      uint8_t value = data[channel_number - 1];
      channel->update_value(value);
    }
  }
}

void ArtNetReceiver::on_dmx_frame_callback(uint16_t universe, uint16_t length,
                                           uint8_t sequence, uint8_t *data) {
  if (instance_ != nullptr) {
    instance_->on_dmx_frame(universe, length, sequence, data);
  }
}

} // namespace esphome::artnet_receiver
