# ESPHome ArtNet Receiver Component - Testing Guide

This directory contains a minimal ESPHome YAML file for testing the ArtNet receiver component.

## Quick Start

1. Update the WiFi credentials in `simple-test.yaml`
2. Build and flash to your ESP32:
   ```bash
   esphome run simple-test.yaml
   ```
3. Send Art-Net data to your ESP32's IP address on universe 0
4. Watch the logs to see DMX channel values

## Testing

You can test the component with any Art-Net software. For a quick test, you can use:

- **QLC+** (Free, cross-platform): https://www.qlcplus.org/
- **ArtNetominator** (Windows, simple test tool)
- **Art-Net Controller** (Android app)

Set your Art-Net software to send to the ESP32's IP address on universe 0.

## Expected Output

When Art-Net data is received, you should see log messages like:

```
[D][artnet_channel:20] Channel 1 updated to value: 128
[D][artnet_channel:20] Channel 2 updated to value: 255
[D][artnet_channel:20] Channel 3 updated to value: 64
```

The sensor values will also be available in Home Assistant if you have the API enabled.