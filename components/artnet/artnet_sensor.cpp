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
    // Convert DMX value (0-255) to a float (0.0-255.0) for the sensor
    this->publish_state((float)value);
    ESP_LOGD(TAG, "'%s': Channel %d updated to value: %d",
             this->get_name().c_str(), this->channel_, value);
  }
}

} // namespace esphome::artnet
