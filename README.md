# TelemetryJet Arduino SDK

[![arduino-library-badge](https://www.ardu-badge.com/badge/TelemetryJet.svg?)](https://www.ardu-badge.com/TelemetryJet)
![commit-activity-badge](https://img.shields.io/github/last-commit/telemetryjet/telemetryjet-arduino-sdk)
![license-badge](https://img.shields.io/github/license/telemetryjet/telemetryjet-arduino-sdk)
--- 

The TelemetryJet Arduino SDK is a lightweight, flexible library for communicating with microcontrollers. The Arduino SDK is a wrapper around [MessagePack](https://msgpack.org/index.html) that allows you to send and receive data points using a high-level API.

### Features
- **Bidirectional communication**: The SDK encodes and decodes data with MessagePack over a serial connection, providing a robust telemetry link that requires minimal setup. Easily send or receive data and configure transmission rate, error checking, and other serial communication settings.

- **Pub/sub messaging & caching**: Filter incoming data points, so your microcontroller only stores values you’ve selected. Data you’ve subscribed to caches locally on your device.

- **Easy integration with any software**: The SDK sends pure MessagePack structures. You can easily parse messages in any program using [MessagePack’s language bindings](https://msgpack.org/index.html), or use the TelemetryJet CLI to stream data into other data sources without code.

### Documentation
Full documentation for the TelemetryJet Arduino SDK is provided on the [TelemetryJet Documentation Site](https://docs.telemetryjet.com/arduino_sdk/).

### Compatibility

The TelemetryJet Arduino SDK requires Arduino IDE 1.5.x+.
