#pragma once

#include "artnet.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome::artnet {

class ArtNetSensor : public sensor::Sensor, public Component {
public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_artnet_parent(ArtNet *parent) { this->parent_ = parent; }
  void set_universe(uint16_t universe) { this->universe_ = universe; }
  void set_channel(uint16_t channel) { this->channel_ = channel; }

  uint16_t get_universe() const { return this->universe_; }
  uint16_t get_channel() const { return this->channel_; }

  void update_value(uint8_t value);

protected:
  ArtNet *parent_{nullptr};
  uint16_t universe_{0};
  uint16_t channel_{1};
  uint8_t last_value_{0};
};

} // namespace esphome::artnet
