#include "artnet_output.h"
#include "esphome/core/log.h"

namespace esphome::artnet {

static const char *const TAG = "artnet.output";

// Static members
std::map<ArtNetOutput::UniverseKey, std::vector<uint8_t>>
    ArtNetOutput::dmx_buffers_;
std::vector<ArtNetOutput *> ArtNetOutput::outputs_;
bool ArtNetOutput::needs_update_ = false;

void ArtNetOutput::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ArtNet Output...");

  // Register this output
  register_output(this);

  ESP_LOGCONFIG(TAG, "ArtNet Output setup complete");
}

void ArtNetOutput::dump_config() {
  ESP_LOGCONFIG(TAG, "ArtNet Output:");
  ESP_LOGCONFIG(TAG, "  Destination: %s",
                this->destination_.toString().c_str());
  ESP_LOGCONFIG(TAG, "  Universe: %d", this->universe_);
  ESP_LOGCONFIG(TAG, "  Channel: %d", this->channel_);
  LOG_FLOAT_OUTPUT(this);
}

void ArtNetOutput::set_destination_address(const std::string &address) {
  this->destination_.fromString(address.c_str());
}

void ArtNetOutput::write_state(float state) {
  // Convert float (0.0-1.0) to DMX value (0-255)
  uint8_t value = static_cast<uint8_t>(state * 255.0f);

  if (this->current_value_ != value) {
    this->current_value_ = value;

    // Update the DMX buffer for this universe

    // Mark that we need to send updates
    needs_update_ = true;
  }

  // Send DMX data if needed
  if (needs_update_) {
    send_dmx_data();
    needs_update_ = false;
  }
}

void ArtNetOutput::register_output(ArtNetOutput *output) {
  outputs_.push_back(output);
}

void ArtNetOutput::send_dmx_data() {}

} // namespace esphome::artnet
