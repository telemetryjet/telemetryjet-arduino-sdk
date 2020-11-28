# TelemetryJet Arduino SDK

--- 

The TelemetryJet Arduino SDK is a lightweight library for communicating sensor and control data to or from microcontrollers. The Arduino SDK is designed primary for ease-of-use, with a simple, flexible API for instantly getting data from your embedded device. The SDK provides a high-level abstraction for getting and setting data and events, and handles the low-level functionality of transmitting those events.

The TelemetryJet CLI can natively ingest and stream data from the Arduino SDK without any setup. We also provide bindings for the SDK in several languages for use in your own software.

The TelemetryJet Arduino SDK is an excellent solution if you are interacting with a single microcontroller in your project. The SDK can be used with or without TelemetryJet itself.


### Features
- Bidirectional communication; send commands and/or data points in either direction
- Send and receive data, and cache latest data points on the Arduino SDK
- Cache expiration times
- Send and receive one-off events
- Operates over software or hardware serial, independent of transport mechanism
- 100% free and open source


### Philosophy
The SDK takes an opinionated approach to message parsing and provides high-level functionality.
The TelemetryJet Arduino SDK is built on top of [MessagePack](https://msgpack.org/index.html), which provides an efficient serialization protocol for arbitrary data. On top of MessagePack, the SDK adds
an API for bidirectional communication, a data point cache, one-off event triggers, and more.

### Documentation
Full documentation for the TelemetryJet Arduino SDK is provided on the [TelemetryJet Documentation Site](https://docs.telemetryjet.com/arduino_sdk/).

### Compatibility

The TelemetryJet Arduino SDK requires Arduino IDE 1.5.x+.