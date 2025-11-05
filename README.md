# ESPHome ArtNet Component

A custom ESPHome component that receives and sends Art-Net DMX data over WiFi using the [ArtnetWifi library](https://github.com/rstephan/ArtnetWifi). This component allows you to:
- Receive Art-Net packets and expose individual DMX channels as ESPHome sensors
- Send DMX output via Art-Net to lighting consoles and controllers
- Route Art-Net data to/from physical DMX buses using [esphome-dmx](https://github.com/H3mul/esphome-dmx)
- Integrate with ESPHome's light, switch, and automation systems

## Features

- **Bidirectional Art-Net Support**: Send and receive Art-Net DMX data over WiFi
- **DMX Routing**: Route Art-Net universes to physical DMX buses and vice versa
- **Universe Support**: Supports multiple Art-Net universes (0-15)
- **Channel Monitoring**: Exposes individual DMX channels as ESPHome sensors
- **Output Channels**: Control lights and devices via Art-Net output
- **Home Assistant Integration**: Full support for Home Assistant API
- **Real-time Updates**: Automatic handling of Art-Net packet processing with configurable update periods

## Installation

### Option 1: Git Repository (Recommended)

Add this component to your ESPHome configuration using the `external_components` feature:

```yaml
external_components:
  - source: github://h3mul/esphome-artnet@main
```

### Option 2: Local Installation

1. Clone this repository to your ESPHome project directory
2. Reference the local components directory in your YAML:

```yaml
external_components:
  - source: components
```

## Development Setup

This project uses clangd for C++ code completion and diagnostics in the component development.

### Clangd Configuration

1. Ensure clangd is installed and configured in your editor (VS Code has the LLVM clangd extension).
2. Run the task to generate the compilation database:
   ```bash
   task compiledb
   ```
   This generates `compile_commands.json` from the ESPHome build and symlinks it to the project root for clangd to use.
3. The `.clangd` configuration file is already set up to use the compilation database from the root directory, providing accurate code completion and error checking for the C++ components.

## Dependencies

- **WiFi**: Required for Art-Net communication
- **ArtnetWifi Library**: Automatically installed (v1.6.1)
- **esphome-dmx** (Optional): Required for DMX routing features

## Configuration

### Basic Configuration

```yaml
artnet:
  id: artnet_component
  
  # Optional: Configure Art-Net output
  output:
    address: 10.1.1.10      # Target IP for Art-Net packets
    flush_period: 100ms     # How often to send updates
  
  # Optional: Configure routing to/from DMX buses
  route:
    artnet_to_dmx:
      - dmx_id: dmx_bus_a
        universe: 1
    dmx_to_artnet:
      - dmx_id: dmx_bus_b
        universe: 2
```

### Configuration Variables

#### `artnet` Component

- **id** (*Required*, [ID](https://esphome.io/guides/configuration-types.html#config-id)): Unique ID for the ArtNet component.
- **output** (*Optional*, [Output Configuration](#output-configuration)): Configure Art-Net output settings.
- **route** (*Optional*, [Route Configuration](#route-configuration)): Configure DMX routing.

#### Output Configuration

- **address** (*Optional*, IPv4 address): Destination IP for outgoing Art-Net packets. If not set, packets are not sent.
- **flush_period** (*Optional*, [time](https://esphome.io/guides/configuration-types.html#config-time)): How frequently to send Art-Net output updates. Defaults to `100ms`.

#### Route Configuration

- **artnet_to_dmx** (*Optional*, list): Route Art-Net universes to physical DMX buses.
  - **dmx_id** (*Required*, [reference](https://esphome.io/guides/configuration-types.html#config-id)): Reference to a DMX bus component configured in [esphome-dmx](https://github.com/H3mul/esphome-dmx).
  - **universe** (*Required*, int): Art-Net universe to route (0-15).

- **dmx_to_artnet** (*Optional*, list): Route physical DMX data to Art-Net universes.
  - **dmx_id** (*Required*, [reference](https://esphome.io/guides/configuration-types.html#config-id)): Reference to a DMX bus component configured in [esphome-dmx](https://github.com/H3mul/esphome-dmx).
  - **universe** (*Required*, int): Art-Net universe to send data to (0-15).

### Sensor Platform

Expose Art-Net DMX values as sensors:

```yaml
sensor:
  - platform: artnet
    artnet_id: artnet_component    # Reference to the artnet component
    id: artnet_dimmer_1
    name: "DMX Channel 1"
    universe: 0                    # Art-Net universe (0-15)
    channel: 1                     # DMX channel (1-512)
```

**Configuration Variables:**
- **artnet_id** (*Optional*, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ArtNet component to use. Defaults to the first ArtNet component.
- **universe** (*Required*, int): Art-Net universe (0-15).
- **channel** (*Required*, int): DMX channel (1-512).
- All standard [Sensor](https://esphome.io/components/sensor/index.html) configuration options.

### Output Platform

Control lights and devices via Art-Net:

```yaml
output:
  - platform: artnet
    artnet_id: artnet_component    # Reference to the artnet component
    id: artnet_red_output
    universe: 0                    # Art-Net universe (0-15)
    channel: 1                     # DMX channel (1-512)
```

**Configuration Variables:**
- **artnet_id** (*Optional*, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ArtNet component to use. Defaults to the first ArtNet component.
- **universe** (*Required*, int): Art-Net universe (0-15).
- **channel** (*Required*, int): DMX channel (1-512).
- All standard [Output](https://esphome.io/components/output/index.html) configuration options.

## Usage Examples

### Simple Dimmer Control

```yaml
artnet:
  id: artnet_component

sensor:
  - platform: artnet
    id: dimmer_1
    name: "Main Dimmer"
    universe: 0
    channel: 1

light:
  - platform: monochromatic
    name: "Main Light"
    output: main_output

output:
  - platform: esp32_ledc
    pin: GPIO5
    id: main_output

automation:
  - trigger:
      sensor.on_value:
        id: dimmer_1
    action:
      - light.turn_on:
          id: main_light
          brightness: !lambda |-
            return id(dimmer_1).state / 255.0;
```

### RGB Light Control with Art-Net

```yaml
artnet:
  id: artnet_component

sensor:
  - platform: artnet
    id: rgb_red
    name: "RGB Red"
    universe: 0
    channel: 1

  - platform: artnet
    id: rgb_green
    name: "RGB Green"
    universe: 0
    channel: 2

  - platform: artnet
    id: rgb_blue
    name: "RGB Blue"
    universe: 0
    channel: 3

light:
  - platform: rgb
    name: "RGB Strip"
    red: red_output
    green: green_output
    blue: blue_output

output:
  - platform: esp32_ledc
    pin: GPIO23
    id: red_output
  - platform: esp32_ledc
    pin: GPIO22
    id: green_output
  - platform: esp32_ledc
    pin: GPIO21
    id: blue_output

automation:
  - trigger:
      - sensor.on_value: rgb_red
      - sensor.on_value: rgb_green
      - sensor.on_value: rgb_blue
    action:
      - light.turn_on:
          id: rgb_strip
          red: !lambda "return id(rgb_red).state / 255.0;"
          green: !lambda "return id(rgb_green).state / 255.0;"
          blue: !lambda "return id(rgb_blue).state / 255.0;"
```

### DMX Routing (Art-Net to Physical DMX)

Route incoming Art-Net universe 1 to a physical DMX output:

```yaml
external_components:
  - source: components
  - source: ../esphome-dmx/components

dmx:
  - id: dmx_out
    mode: send
    enable_pin: GPIO18
    tx_pin: GPIO16
    rx_pin: GPIO17
    dmx_port_id: 1

artnet:
  route:
    artnet_to_dmx:
      - dmx_id: dmx_out
        universe: 1
```

Now Art-Net packets received on universe 1 will automatically be sent out on the physical DMX bus.

### Bidirectional DMX/Art-Net Bridge

Bridge physical DMX and Art-Net in both directions:

```yaml
dmx:
  - id: dmx_send
    mode: send
    enable_pin: GPIO18
    tx_pin: GPIO16
    rx_pin: GPIO17
    dmx_port_id: 1

  - id: dmx_receive
    mode: receive
    enable_pin: GPIO23
    tx_pin: GPIO21
    rx_pin: GPIO22
    dmx_port_id: 2

artnet:
  output:
    address: 10.1.1.10      # Send Art-Net to this address
    flush_period: 10ms

  route:
    artnet_to_dmx:
      - dmx_id: dmx_send
        universe: 1
    dmx_to_artnet:
      - dmx_id: dmx_receive
        universe: 2
```

This configuration:
1. Sends Art-Net data from universe 1 to the physical DMX output (`dmx_send`)
2. Receives DMX data from `dmx_receive` and forwards it as Art-Net universe 2 to `10.1.1.10`

## Art-Net Software Compatibility

This component has been tested with the following Art-Net software:

- **QLC+** (Open Source DMX/Art-Net controller)
- **Avolites Titan** (Professional lighting console software)
- **GrandMA onPC** (MA Lighting console software)
- **LightKey** (Mac lighting software)
- **Freestyler DMX** (Windows DMX software)

## Troubleshooting

### Common Issues

1. **No Art-Net data received**
   - Ensure WiFi is connected and working
   - Check that the Art-Net software is sending to the correct IP address
   - Verify the universe number matches between sender and receiver
   - Check firewall settings on your network

2. **Sensor values not updating**
   - Verify the universe and channel numbers are correct (channels 1-512)
   - Check the ESPHome logs for any error messages
   - Ensure the Art-Net sender is actually transmitting data

3. **DMX routing not working**
   - Verify the DMX component is properly configured in esphome-dmx
   - Check that the `dmx_id` in the route matches the DMX component ID
   - Ensure the DMX component is set to the correct mode (send/receive)
   - Check the ESPHome logs for routing information

4. **Component fails to compile**
   - Make sure you have the latest version of ESPHome
   - Check that the external component source is accessible
   - Verify WiFi component is included in your configuration
   - If using DMX routing, ensure esphome-dmx component is accessible

### Debugging

Enable verbose logging to see Art-Net packet information:

```yaml
logger:
  level: VERBOSE
  logs:
    artnet: VERBOSE
    artnet.sensor: VERBOSE
    artnet.output: VERBOSE
```

### Performance Notes

- The component processes Art-Net packets in the main loop
- Each channel update triggers a sensor state change
- For high-frequency DMX data (44Hz), consider limiting the number of sensors
- The ESP32 can typically handle 20-50 channels without performance issues
- DMX routing adds minimal overhead (~1ms per routed universe)

## Technical Details

### Art-Net Protocol

- **Protocol**: Art-Net v4
- **Universe Range**: 0-15 (standard Art-Net universe numbering)
- **Channel Range**: 1-512 (standard DMX channel numbering)
- **Data Format**: 8-bit values (0-255)
- **Refresh Rate**: Up to 44Hz (typical DMX refresh rate)

### Memory Usage

- Base component: ~3KB RAM
- Per sensor/output: ~100 bytes RAM
- Per DMX route: ~8 bytes RAM
- ArtnetWifi library: ~4KB RAM

### Network Requirements

- **Protocol**: UDP
- **Port**: 6454 (standard Art-Net port)
- **Broadcast**: Listens for broadcast packets on local network
- **Bandwidth**: ~1KB per universe update (typical)

## Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Credits

- Based on the [ArtnetWifi library](https://github.com/rstephan/ArtnetWifi) by rstephan
- DMX integration via [esphome-dmx](https://github.com/H3mul/esphome-dmx)
- Designed for use with [ESPHome](https://esphome.io/)
- Art-Net protocol specification by Artistic Licence