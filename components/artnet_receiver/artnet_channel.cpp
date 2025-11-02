#include "artnet_channel.h"
#include "esphome/core/log.h"

namespace esphome {
namespace artnet_receiver {

static const char *const TAG = "artnet_channel";

void ArtNetChannel::dump_config() {
  ESP_LOGCONFIG(TAG, "ArtNet Channel:");
  ESP_LOGCONFIG(TAG, "  Channel Number: %d", this->channel_number_);
  ESP_LOGCONFIG(TAG, "  Name: %s", this->get_name().c_str());
}

void ArtNetChannel::update_value(uint8_t value) {
  if (this->last_value_ != value) {
    this->last_value_ = value;
    // Convert DMX value (0-255) to a float (0.0-255.0) for the sensor
    this->publish_state((float)value);
    ESP_LOGD(TAG, "Channel %d updated to value: %d", this->channel_number_, value);
  }
}

}  // namespace artnet_receiver
}  // namespace esphome