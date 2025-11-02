#pragma once

#include "artnet_channel.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <ArtnetWifi.h>
#include <WiFi.h>
#include <vector>

namespace esphome::artnet_receiver {

static const char *const TAG = "artnet_receiver";

class ArtNetReceiver : public Component {
public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override {
    return setup_priority::AFTER_WIFI;
  }

  void set_artnet_universe(uint16_t universe) {
    this->artnet_universe_ = universe;
  }
  void add_channel(ArtNetChannel *channel) {
    this->channels_.push_back(channel);
  }

protected:
  void on_dmx_frame(uint16_t universe, uint16_t length, uint8_t sequence,
                    uint8_t *data);
  static void on_dmx_frame_callback(uint16_t universe, uint16_t length,
                                    uint8_t sequence, uint8_t *data);

  ArtnetWifi *artnet_;
  uint16_t artnet_universe_{0};
  std::vector<ArtNetChannel *> channels_;
  static ArtNetReceiver *instance_;
};

} // namespace esphome::artnet_receiver
