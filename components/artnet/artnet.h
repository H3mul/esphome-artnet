#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <ArtnetWifi.h>
#include <WiFi.h>
#include <vector>

namespace esphome::artnet {

class ArtNetSensor; // Forward declaration

class ArtNet : public Component {
public:
  static ArtNet *get_instance() { return instance_; }

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override {
    return setup_priority::AFTER_WIFI;
  }

protected:
  static ArtnetWifi *artnet_;
  static ArtNet *instance_;
  static std::vector<ArtNetSensor *> sensors_;

  // Callback for incoming Art-Net frames
  virtual void on_artnet_frame(uint16_t universe, uint16_t length,
                               uint8_t sequence, uint8_t *data);

  // Static callback function for ArtnetWifi library
  static void artnet_callback(uint16_t universe, uint16_t length,
                              uint8_t sequence, uint8_t *data);

public:
  static void register_sensor(ArtNetSensor *sensor);
};

} // namespace esphome::artnet
