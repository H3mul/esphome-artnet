#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <ArtnetWifi.h>
#include <WiFi.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

#ifdef USE_DMX_COMPONENT
namespace esphome::dmx {
class DMXComponent; // Forward declaration
}
#endif

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

  void set_continuous_output(bool continuous_output) {
    this->continuous_output_ = continuous_output;
  }

  void set_name_short(const std::string &name_short) {
    this->name_short_ = name_short;
  }
  const std::string &get_name_short() const { return this->name_short_; }

  void set_name_long(const std::string &name_long) {
    this->name_long_ = name_long;
  }
  const std::string &get_name_long() const { return this->name_long_; }

  void set_output_net(uint8_t net) {
    if (net <= 127) {
      this->output_net_ = net;
    }
  }
  uint8_t get_output_net() const { return this->output_net_; }

  void set_output_subnet(uint8_t subnet) {
    if (subnet <= 15) {
      this->output_subnet_ = subnet;
    }
  }
  uint8_t get_output_subnet() const { return this->output_subnet_; }

  void set_net(uint8_t net) {
    if (net <= 127) {
      this->net_ = net;
    }
  }
  uint8_t get_net() const { return this->net_; }

  void set_subnet(uint8_t subnet) {
    if (subnet <= 15) {
      this->subnet_ = subnet;
    }
  }
  uint8_t get_subnet() const { return this->subnet_; }

#ifdef USE_DMX_COMPONENT
  void add_artnet_to_dmx_route(esphome::dmx::DMXComponent *dmx_component,
                               uint16_t universe) {
    artnet_to_dmx_routes_.push_back(std::make_pair(dmx_component, universe));
  }

  void add_dmx_to_artnet_route(esphome::dmx::DMXComponent *dmx_component,
                               uint16_t universe) {
    dmx_to_artnet_routes_.push_back(std::make_pair(dmx_component, universe));
  }
#endif

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
  uint8_t output_net_{0};
  uint8_t output_subnet_{0};
  uint8_t net_{0};
  uint8_t subnet_{0};
  bool continuous_output_{false};
  std::string name_short_{};
  std::string name_long_{};
  IPAddress poll_reply_sender_ip_;
  uint32_t last_poll_reply_time_{0};
  uint32_t poll_reply_delay_ms_{0};
  uint16_t poll_response_counter_{0};
  static const uint32_t POLL_REPLY_MAX_DELAY_MS =
      1000; // Random delay up to 1s before sending reply

  void send_outputs_data();

  // Callback for incoming Art-Net frames
  virtual void on_artnet_frame(uint16_t universe, uint16_t length,
                               uint8_t sequence, uint8_t *data);

  // Static callback function for ArtnetWifi library
  static void artnet_callback(uint16_t universe, uint16_t length,
                              uint8_t sequence, uint8_t *data);

#ifdef USE_DMX_COMPONENT
  std::vector<std::pair<esphome::dmx::DMXComponent *, uint16_t>>
      artnet_to_dmx_routes_;
  std::vector<std::pair<esphome::dmx::DMXComponent *, uint16_t>>
      dmx_to_artnet_routes_;
#endif

  void route_dmx_to_artnet();
  void route_artnet_to_dmx(uint8_t universe, uint8_t *data, uint16_t length);
  void send_poll_reply();
};

} // namespace esphome::artnet
