# ESPHome ArtNet Receiver Component

A custom ESPHome component that receives Art-Net DMX data over WiFi using the [ArtnetWifi library](https://github.com/rstephan/ArtnetWifi). This component allows you to receive Art-Net packets and expose individual DMX channels as ESPHome sensors, which can then be used to control lights, switches, or other components.

## Features

- Receives Art-Net DMX data over WiFi
- Supports universe selection (0-15)
- Exposes individual DMX channels as ESPHome sensors
- Easy integration with ESPHome's light, switch, and automation systems
- Automatic handling of Art-Net packet processing
- Real-time DMX value updates

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

- **WiFi**: Required for receiving Art-Net packets
- **ArtnetWifi Library**: Automatically installed (v1.5.1)

## Configuration

### Basic Configuration

```yaml
artnet_receiver:
  id: artnet_receiver
  artnet_universe: 0
  channels:
    - channel_number: 1
      name: "Dimmer 1"
      id: dmx_ch1
    - channel_number: 2
      name: "Dimmer 2" 
      id: dmx_ch2
```

### Configuration Variables

#### `artnet_receiver` Component

- **id** (*Required*, [ID](https://esphome.io/guides/configuration-types.html#config-id)): Unique ID for the ArtNet receiver component.
- **artnet_universe** (*Optional*, int): The Art-Net universe to listen to. Range: 0-15. Defaults to `0`.
- **channels** (*Optional*, list): List of DMX channels to monitor and expose as sensors.

#### Channel Configuration

Each channel in the `channels` list supports the following options:

- **channel_number** (*Required*, int): DMX channel number to monitor. Range: 1-512.
- **name** (*Required*, string): Human-readable name for the sensor.
- **id** (*Required*, [ID](https://esphome.io/guides/configuration-types.html#config-id)): Unique ID for the sensor.
- All standard [Sensor](https://esphome.io/components/sensor/index.html) configuration options.

## Usage Examples

### Simple Dimmer Control

```yaml
artnet_receiver:
  id: artnet_receiver
  artnet_universe: 0
  channels:
    - channel_number: 1
      name: "Main Dimmer"
      id: main_dimmer

# Use the DMX value to control a light
light:
  - platform: monochromatic
    name: "Main Light"
    output: main_output

output:
  - platform: esp32_ledc
    pin: GPIO5
    id: main_output

# Automation to sync DMX value with light brightness
automation:
  - trigger:
      - sensor.on_value:
          id: main_dimmer
    action:
      - light.turn_on:
          id: main_light
          brightness: !lambda |-
            return id(main_dimmer).state / 255.0;
```

### RGB Light Control

```yaml
artnet_receiver:
  id: artnet_receiver
  artnet_universe: 0
  channels:
    - channel_number: 1
      name: "RGB Red"
      id: rgb_red
    - channel_number: 2
      name: "RGB Green"
      id: rgb_green
    - channel_number: 3
      name: "RGB Blue"
      id: rgb_blue

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

# Sync RGB values from DMX
automation:
  - trigger:
      - sensor.on_value:
          id: rgb_red
      - sensor.on_value:
          id: rgb_green
      - sensor.on_value:
          id: rgb_blue
    action:
      - light.turn_on:
          id: rgb_strip
          red: !lambda "return id(rgb_red).state / 255.0;"
          green: !lambda "return id(rgb_green).state / 255.0;"
          blue: !lambda "return id(rgb_blue).state / 255.0;"
```

### Switch Control

```yaml
artnet_receiver:
  id: artnet_receiver
  artnet_universe: 0
  channels:
    - channel_number: 10
      name: "Switch Trigger"
      id: switch_channel

switch:
  - platform: gpio
    pin: GPIO18
    name: "DMX Controlled Switch"
    id: dmx_switch

# Turn switch on when DMX value > 127, off when <= 127
automation:
  - trigger:
      - sensor.on_value:
          id: switch_channel
          above: 127
    action:
      - switch.turn_on: dmx_switch
      
  - trigger:
      - sensor.on_value:
          id: switch_channel
          below: 128
    action:
      - switch.turn_off: dmx_switch
```

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
   - Verify the channel numbers are correct (1-512)
   - Check the ESPHome logs for any error messages
   - Ensure the Art-Net sender is actually transmitting data

3. **Component fails to compile**
   - Make sure you have the latest version of ESPHome
   - Check that the external component source is accessible
   - Verify WiFi component is included in your configuration

### Debugging

Enable verbose logging to see Art-Net packet information:

```yaml
logger:
  level: VERBOSE
  logs:
    artnet_receiver: VERBOSE
```

### Performance Notes

- The component processes Art-Net packets in the main loop
- Each channel update triggers a sensor state change
- For high-frequency DMX data (44Hz), consider limiting the number of channels
- The ESP32 can typically handle 20-50 channels without performance issues

## Technical Details

### Art-Net Protocol

- **Protocol**: Art-Net v4
- **Universe Range**: 0-15 (standard Art-Net universe numbering)
- **Channel Range**: 1-512 (standard DMX channel numbering)
- **Data Format**: 8-bit values (0-255)
- **Refresh Rate**: Up to 44Hz (typical DMX refresh rate)

### Memory Usage

- Base component: ~2KB RAM
- Per channel: ~100 bytes RAM
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
- Designed for use with [ESPHome](https://esphome.io/)
- Art-Net protocol specification by Artistic Licence