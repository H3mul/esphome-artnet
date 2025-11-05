#include "artnet_output.h"
#include "esphome/core/log.h"

namespace esphome::artnet {

static const char *const TAG = "artnet.output";

void ArtNetOutput::setup() {
  // Register this output with the ArtNet component
  ArtNet::register_output(this);
  ESP_LOGCONFIG(TAG, "Setting up ArtNet Output...");
  ESP_LOGCONFIG(TAG, "ArtNet Output setup complete");
}

void ArtNetOutput::dump_config() {
  ESP_LOGCONFIG(TAG, "ArtNet Output:");
  ESP_LOGCONFIG(TAG, "  Universe: %d", this->universe_);
  ESP_LOGCONFIG(TAG, "  Channel: %d", this->channel_);
  LOG_FLOAT_OUTPUT(this);
}

void ArtNetOutput::write_state(float state) {
  // Convert float (0.0-1.0) to DMX value (0-255)
  uint8_t current_value_ = static_cast<uint8_t>(state * 255.0f);

  // Update the DMX buffer for this universe

  // Send DMX data if needed
}

} // namespace esphome::artnet
