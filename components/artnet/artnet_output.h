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
  void set_destination_address(const std::string &address);
  void set_universe(uint16_t universe) { this->universe_ = universe; }
  void set_channel(uint16_t channel) { this->channel_ = channel; }

  uint16_t get_universe() const { return this->universe_; }
  uint16_t get_channel() const { return this->channel_; }
  const IPAddress &get_destination() const { return this->destination_; }

protected:
  void write_state(float state) override;
  
  // Helper to send DMX data
  static void send_dmx_data();
  static void register_output(ArtNetOutput *output);

  ArtNet *parent_{nullptr};
  IPAddress destination_;
  uint16_t universe_{0};
  uint16_t channel_{1};
  uint8_t current_value_{0};
  
  // Structure to hold DMX data per universe per destination
  struct UniverseKey {
    IPAddress destination;
    uint16_t universe;
    
    bool operator<(const UniverseKey &other) const {
      if (destination != other.destination) {
        return destination < other.destination;
      }
      return universe < other.universe;
    }
  };
  
  static std::map<UniverseKey, std::vector<uint8_t>> dmx_buffers_;
  static std::vector<ArtNetOutput *> outputs_;
  static bool needs_update_;
};

}  // namespace esphome::artnet
