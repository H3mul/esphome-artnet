#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <ArtnetWifi.h>
#include <WiFi.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace esphome::dmx {
class DMXComponent; // Forward declaration
}

namespace esphome::artnet {

class ArtNetSensor; // Forward declaration
class ArtNetOutput; // Forward declaration

class ArtNet : public Component {
public:
  static ArtNet *get_instance() { return instance_; }

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override {
    return setup_priority::AFTER_WIFI;
  }

  void set_output_address(const std::string &address) {
    if (!address.empty()) {
      this->output_address_ = IPAddress(address.c_str());
    }
  }
  const IPAddress &get_output_address() const { return this->output_address_; }

  void set_flush_period(uint32_t flush_period) {
    this->flush_period_ms_ = flush_period;
  }

  void add_artnet_to_dmx_route(esphome::dmx::DMXComponent *dmx_component,
                               uint16_t universe) {
    artnet_to_dmx_routes_.push_back(std::make_pair(dmx_component, universe));
  }

  void add_dmx_to_artnet_route(esphome::dmx::DMXComponent *dmx_component,
                               uint16_t universe) {
    dmx_to_artnet_routes_.push_back(std::make_pair(dmx_component, universe));
  }

  static void register_sensor(ArtNetSensor *sensor);
  static void register_output(ArtNetOutput *output);

protected:
  static ArtnetWifi *artnet_;
  static ArtNet *instance_;
  static std::vector<ArtNetSensor *> sensors_;
  static std::map<uint16_t, std::vector<ArtNetOutput *>> outputs_per_universe_;

  IPAddress output_address_;
  uint32_t flush_period_ms_{100};
  uint32_t last_flush_time_{0};

  std::vector<std::pair<esphome::dmx::DMXComponent *, uint16_t>>
      artnet_to_dmx_routes_;
  std::vector<std::pair<esphome::dmx::DMXComponent *, uint16_t>>
      dmx_to_artnet_routes_;

  void send_outputs_data();

  void route_dmx_to_artnet();
  void route_artnet_to_dmx(uint16_t universe, uint16_t length, uint8_t sequence,
                           uint8_t *data);

  // Callback for incoming Art-Net frames
  virtual void on_artnet_frame(uint16_t universe, uint16_t length,
                               uint8_t sequence, uint8_t *data);

  // Static callback function for ArtnetWifi library
  static void artnet_callback(uint16_t universe, uint16_t length,
                              uint8_t sequence, uint8_t *data);
};

} // namespace esphome::artnet
