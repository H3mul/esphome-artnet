#pragma once

#include "artnet.h"
#include "esphome/components/output/float_output.h"
#include "esphome/core/component.h"
#include <IPAddress.h>
#include <map>
#include <vector>

namespace esphome::artnet {

class ArtNetOutput : public output::FloatOutput, public Component {
public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_artnet_parent(ArtNet *parent) { this->parent_ = parent; }
  void set_universe(uint16_t universe) { this->universe_ = universe; }
  void set_channel(uint16_t channel) { this->channel_ = channel; }

  uint16_t get_universe() const { return this->universe_; }
  uint16_t get_channel() const { return this->channel_; }
  uint8_t get_current_value() const { return this->current_value_; }

protected:
  void write_state(float state) override;

  ArtNet *parent_{nullptr};
  uint16_t universe_{0};
  uint16_t channel_{1};
  uint8_t current_value_{0};
};

} // namespace esphome::artnet
