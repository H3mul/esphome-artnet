#include "artnet_sensor.h"
#include "esphome/core/log.h"

namespace esphome::artnet {

static const char *const TAG = "artnet.sensor";

void ArtNetSensor::setup() {
  // Register this sensor with the ArtNet component
  ArtNet::register_sensor(this);
}

void ArtNetSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "ArtNet Sensor:");
  ESP_LOGCONFIG(TAG, "  Universe: %d", this->universe_);
  ESP_LOGCONFIG(TAG, "  Channel: %d", this->channel_);
  LOG_SENSOR("  ", "Sensor", this);
}

void ArtNetSensor::update_value(uint8_t value) {
  if (this->last_value_ != value) {
    this->last_value_ = value;
    this->publish_state((float)value);
  }
}

} // namespace esphome::artnet
