# TelemetryJet Arduino SDK

[![arduino-library-badge](https://www.ardu-badge.com/badge/TelemetryJet.svg?)](https://www.ardu-badge.com/TelemetryJet)
![version-badge](https://img.shields.io/github/v/release/telemetryjet/telemetryjet-arduino-sdk)
![commit-activity-badge](https://img.shields.io/github/last-commit/telemetryjet/telemetryjet-arduino-sdk)
![license-badge](https://img.shields.io/github/license/telemetryjet/telemetryjet-arduino-sdk)

The TelemetryJet Arduino SDK is a lightweight library for communicating with microcontrollers using a high-level API. Messages are sent and received as [MessagePack](https://msgpack.org/index.html)-encoded data, with packet framing and error detection added around the data. The Arduino SDK is part of the TelemetryJet platform, a set of open-source tools to collect, analyze and share hardware data.

### Features
- **Simple, Bidirectional Communication**: The SDK provides a bidirectional telemetry link with an easy to use, high-level API. Define "Dimension" objects, used as a wrapper for getting/setting data points. The underlying data points are automatically transmitted and received over the serial connection.

- **Packet Framing & Error Correction**: The SDK defines an efficient packet encoding, with packet framing, error correction, and other features for data transmission integrity  included out of the box. Easily configure transmission rate and other communication settings. 

- **Data Caching and Expiration**: Filter incoming data points, so your microcontroller only stores values you’ve selected. Data you’ve subscribed to caches locally on your device. Configure data points to expire after a time period, providing automatic fallback for control signals when using an unreliable connection such as radio.

- **Strong typing**: Type information is transmitted with data points. Transmit integer or floating-point numerical data points while preserving resolution. Only store and transmit exactly the amount of data required by your value type.

- **Easy integration with any software**: The SDK sends MessagePack structures, framed with a checksum and packet delimiter. You can easily parse messages in any program using [MessagePack’s language bindings](https://msgpack.org/index.html), or use the [TelemetryJet CLI](https://github.com/telemetryjet/telemetryjet-cli) to stream data into other data sources without code.

### Example
Read two analog input values, and send their values at 10Hz over the serial connection.
```c++
#include <TelemetryJet.h>

// Initialize an instance of the TelemetryJet SDK.
// Configure transmission over Serial port, with an interval of 100ms.
TelemetryJet telemetry(&Serial, 100);

// Create a dimension storing the sensor input values
Dimension sensorValue1 = telemetry.createDimension(1);
Dimension sensorValue2 = telemetry.createDimension(2);

void setup() {
  // Initialize the serial stream.
  Serial.begin(115200);
  while (!Serial);
}

void loop() {
  // Process new data if available, and periodically send data points every 100ms.
  telemetry.update();
  
  // Update the dimensions with the analog input values.
  // Store as a 16-bit unsigned integer.
  sensorValue1.setUInt16(analogRead(A0));
  sensorValue2.setUInt16(analogRead(A1));
}
```

### Packet Specification
At the user-specified transmit interval, the SDK will send a packet for each data point that has an updated value since the last transmission. 
Data is received with the same structure.

|_size_|1 byte  |1 byte      |1-3 bytes*        |1 byte              |1-9 bytes*|1 byte             |
|:-----|:-------|:-----------|:----------------|:-------------------|:--------|:------------------|
|_name_|checksum|padding & mode flags|dimension ID     |value type          |value    |packet frame marker|
|_description_|1-byte checksum of packet. To validate, sum of all bytes in a packet should be 0xFF|Pads packet to ensure checksum is never 0x0. In future versions, will transmits low-priority data for remote debugging.|MessagePack-encoded unsigned 16-bit integer representing the dimension ID, identifying this data point|MessagePack-encoded unsigned 8-bit integer representing the value type|MessagePack-encoded value, as a boolean, 8-64 bit integer, or 32-64 bit float.|0x0 byte representing the end of packet|

All packets are encoded using [Consistent Overhead Byte Stuffing](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing), meaning that the only byte with a value of 0x0 received will be the end of packet marker. 

[\*] Byte sizes for MessagePack-encoded data are defined in the MessagePack specification: https://github.com/msgpack/msgpack/blob/master/spec.md#type-system. In MessagePack, values are encoded using the minimal possible space. Low-value unsigned integers, for example, will be stored in a single byte. With this encoding, the minimum size of a packet is 6 bytes.

### Whitepaper
For an overview of the motivation and design behind the TelemetryJet Arduino SDK, read our Whitepaper: "A Lightweight Telemetry Protocol for Hardware Sensor Data". (*Coming Soon!*). 

### Documentation
Full documentation for the TelemetryJet Arduino SDK is provided on the [TelemetryJet Documentation Site](https://docs.telemetryjet.com/arduino_sdk/).

### Compatibility

The TelemetryJet Arduino SDK requires Arduino IDE 1.5.x+.
