#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome::artnet_receiver {

class ArtNetChannel : public sensor::Sensor, public Component {
public:
  void setup() override {}
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_channel_number(uint16_t channel) { this->channel_number_ = channel; }
  uint16_t get_channel_number() const { return this->channel_number_; }

  void update_value(uint8_t value);

protected:
  uint16_t channel_number_{1};
  uint8_t last_value_{0};
};

} // namespace esphome::artnet_receiver
