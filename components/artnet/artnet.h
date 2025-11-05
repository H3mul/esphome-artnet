#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <ArtnetWifi.h>
#include <WiFi.h>
#include <map>
#include <string>
#include <vector>

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

  static void register_sensor(ArtNetSensor *sensor);
  static void register_output(ArtNetOutput *output);

protected:
  static ArtnetWifi *artnet_;
  static ArtNet *instance_;
  static std::vector<ArtNetSensor *> sensors_;
  static std::map<uint16_t, std::vector<ArtNetOutput *>> outputs_per_universe_;

  IPAddress output_address_;

  void send_outputs_data();

  // Callback for incoming Art-Net frames
  virtual void on_artnet_frame(uint16_t universe, uint16_t length,
                               uint8_t sequence, uint8_t *data);

  // Static callback function for ArtnetWifi library
  static void artnet_callback(uint16_t universe, uint16_t length,
                              uint8_t sequence, uint8_t *data);
};

} // namespace esphome::artnet
