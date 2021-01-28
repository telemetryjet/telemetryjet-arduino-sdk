# TelemetryJet Arduino SDK

[![arduino-library-badge](https://www.ardu-badge.com/badge/TelemetryJet.svg?)](https://www.ardu-badge.com/TelemetryJet)
![version-badge](https://img.shields.io/github/v/release/telemetryjet/telemetryjet-arduino-sdk)
![commit-activity-badge](https://img.shields.io/github/last-commit/telemetryjet/telemetryjet-arduino-sdk)
![license-badge](https://img.shields.io/github/license/telemetryjet/telemetryjet-arduino-sdk)

The TelemetryJet Arduino SDK is a lightweight library for communicating with microcontrollers using a high-level API. Messages are sent and received as [MessagePack](https://msgpack.org/index.html)-encoded data, with packet framing and error detection added around the data. The Arduino SDK is part of the TelemetryJet platform, a set of open-source tools to collect, analyze and share hardware data.

# Features
- **Simple, Bidirectional Communication**: The SDK provides a bidirectional telemetry link with an easy to use, high-level API. Define "Dimension" objects, used as a wrapper for getting/setting data points. The underlying data points are automatically transmitted and received over the serial connection.

- **Packet Framing & Error Correction**: The SDK defines an efficient packet encoding, with packet framing, error correction, and other features for data transmission integrity  included out of the box. Easily configure transmission rate and other communication settings. 

- **Data Caching and Expiration**: Filter incoming data points, so your microcontroller only stores values you’ve selected. Data you’ve subscribed to caches locally on your device. Configure data points to expire after a time period, providing automatic fallback for control signals when using an unreliable connection such as radio.

- **Strong typing**: Type information is transmitted with data points. Transmit integer or floating-point numerical data points while preserving resolution. Only store and transmit exactly the amount of data required by your value type.

- **Easy integration with any software**: The SDK sends MessagePack structures, framed with a checksum and packet delimiter. You can easily parse messages in any program using [MessagePack’s language bindings](https://msgpack.org/index.html), or use the [TelemetryJet CLI](https://github.com/telemetryjet/telemetryjet-cli) to stream data into other data sources without code.

# Example
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
More detailed examples are provided in the `examples/` directory, and are distributed with the Arduino library. 

# Usage
## Configure Telemetry Instance
To use the library, create an instance of `TelemetryJet`, which holds the data processing logic. To create an instance, call the constructor with a reference to the [Arduino Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/) to transmit the data over, and a transmission rate in milliseconds.

```c++
// Initialize an instance of the TelemetryJet SDK.
// Configure transmission over Serial port, with an interval of 100ms.
TelemetryJet telemetry(&Serial, 100);
```

This TelemetryJet instance will read and write data points over the Serial connection, and transmit data points that have updated periodically every 100ms. To process values, you must call `telemetry.update()` in your Arduino program's main loop:

```c++
void loop() {
  // Process new data if available, and periodically send data points every 100ms.
  telemetry.update();
  
  // ...
}
```

Long delays or blocking logic should be avoided in the main loop, to allow `update()` to frequently flush the incoming and outgoing data points.

## Create Dimensions
A "Dimension" is a variable that that can be used to read or write data points. The SDK provides a high-level API to interact with dimensions, and internally handles the nuances of reading and writing packets to the serial stream.


To create a dimension, use the `createDimension` method on your telemetry instance:
```c++
Dimension sensorValue1 = telemetry.createDimension(1);
Dimension sensorValue2 = telemetry.createDimension(2);
```

In the TelemetryJet protocol, dimensions are identified using an unsigned 16-bit integer ID. These IDs contain no semantic meaning; it is up to the user to keep a list of dimension IDs and their uses. Here, we've created two dimensions, with ID 1 and 2.

Once you have created a dimension, you can use methods on the Dimension instances to get and set data points associated with that dimension.

## Reading Values

To read a value, use one of the typed getters. See the [Value Types](#value-types) section below for a full list of types and their methods. For example, to retrieve an integer value:
```c++
// Retrieve a value (Default value is 0 if none exists)
int32_t value = sensorValue1.getInt32();

// Retrieve a value, with a custom default
int32_t value = sensorValue1.getInt32(255);
```

To check if a value is present, use `hasValue` or a typed equivalent:
```c++
// Check if the dimension has any value
sensorValue1.hasValue();

// Check if we received a new value since last interation
sensorValue1.hasNewValue();

// Check if the dimension has a typed value, or compatible smaller value
sensorValue1.hasInt32();

// Check if the dimension has this exact value type ("Exact" mode)
sensorValue1.hasInt32(true);
```

You can also retrieve metadata about a data point such as the type:
```c++
// Get the value type as a DataPointType enum
DataPointType type = sensorValue1.getType();

if (type == DataPointType::BOOLEAN) {
  //...
}
```


## Writing Values

To write a value, use one of the typed getters. See the [Value Types](#value-types) section below for a full list of types and their methods.
```c++
// Write an 8-bit integer
sensorValue1.writeInt8(255);

// Write a 32-bit float
sensorValue2.writeFloat32(234.21);
```

To clear a value:
```c++
sensorValue.clearValue()
```

## Caching & Data Expiration
By default, cached values from input or output data points are stored forever. You can configure an expiration time for a dimension, so an old value is cleared after a timeout period.

For example, to create a throttle signal that would reset after 5000ms:
```c++
Dimension throttle = telemetry.createDimension(3);
throttle.setTimeoutAge(5000);

//...

// Retrieve throttle value. Default to 0 if there is no data received recently.
float32_t throttleValue = throttle.getFloat32(0);

```




## Value Types
All values stored in a dimension are strongly typed. The SDK provides boolean, integer, and floating point data types. Arbitrary string or binary values are not supported -- These values are not common in sensor measurements,
and allow the SDK to provide a fixed bound on memory usage that improves reliability.

|Type name|Enum (ID)|Value Range|Size (Bytes)|Methods|
|---------|-------------|-----------|------------|-------|
|Boolean|`DataPointType::BOOLEAN` (0)|0 to 1|1 byte|`getBool`, `setBool`, `hasBool`|
|Unsigned 8-Bit Integer|`DataPointType::UINT8` (1)|0 to 255|1 byte|`getUInt8`, `setUInt8`, `hasUInt8`|
|Unsigned 16-Bit Integer|`DataPointType::UINT16` (2)|0 to 65,535|1 byte|`getUInt16`, `setUInt16`, `hasUInt16`|
|Unsigned 32-Bit Integer|`DataPointType::UINT32` (3)|0 to 4,294,967,295|1 byte|`getUInt32`, `setUInt32`, `hasUInt32`|
|Unsigned 64-bit Integer|`DataPointType::UINT64` (4)|0 to 18,446,744,073,709,551,615|1 byte|`getUInt64`, `setUInt64`, `hasUInt64`|
|Signed 8-Bit Integer|`DataPointType::INT8` (5)|-128 to 127|1 byte|`getInt8`, `setInt8`, `hasInt8`|
|Signed 16-Bit Integer|`DataPointType::INT16` (6)|-32,768 to 32,767|2 bytes|`getInt16`, `setInt16`, `hasInt16`|
|Signed 32-Bit Integer|`DataPointType::INT32` (7)|-2,147,483,648 to 2,147,483,647|4 bytes|`getInt32`, `setInt32`, `hasInt32`|
|Signed 64-Bit Integer|`DataPointType::INT64` (8)|-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807|8 bytes|`getInt64`, `setInt64`, `hasInt64`|
|32-Bit Float|`DataPointType::FLOAT32` (9)|N/A|8 bytes|`getFloat32`, `setFloat32`, `hasFloat32`|

## Value Conversion

If a getter is called with a compatible larger type than the stored data point, the getter
will convert the value up to the larger type. For example, calling `getInt16` on a dimension with a stored 8-bit integer value will return that value, converted to `int16_t`.

The table below shows the list of compatible getters for each underlying value type.
If the data point has the type on the left, any of the getters on the right will return its value. This hierarchy also applies to value checking using `hasBool`, etc.
|Underlying Type|Compatible Getter|
|---------------|-----------------|
|Bool|`getBool`, `getUInt8`, `getUInt16`, `getUInt32`, `getUInt64`, `getInt8`, `getInt16`, `getInt32`, `getInt64`
|UInt8|`getUInt8`, `getUInt16`, `getUInt32`, `getUInt64`
|UInt16|`getUInt16`, `getUInt32`, `getUInt64`
|UInt32|`getUInt32`, `getUInt64`
|UInt64|`getUInt64`
|Int8|`getInt8`, `getInt16`, `getInt32`, `getInt64`
|Int16|`getInt16`, `getInt32`, `getInt64`
|Int32|`getInt32`, `getInt64`
|Int64|`getInt64`
|Float32|`getFloat32`

# Technical Specification

### Packet Specification
The SDK sends and receives data points in a common binary packet format based on [MessagePack](https://msgpack.org/index.html), with additional features for data validation and packet framing.


|_size_|1 byte  |1 byte      |1-3 bytes*        |1 byte              |1-9 bytes*|1 byte             |
|:-----|:-------|:-----------|:----------------|:-------------------|:--------|:------------------|
|_name_|checksum|padding & mode flags|dimension ID     |value type          |value    |packet frame marker|
|_description_|1-byte checksum of packet. To validate, sum of all bytes the in a packet (including the checksum) should be 0xFF|Padding byte used to shift checksum to never equal 0. Also used to transmit mode flags identifying debug information about the device.|MessagePack-encoded unsigned 16-bit integer representing the dimension ID, identifying this data point *Varying size|MessagePack-encoded unsigned 8-bit integer representing the value type|MessagePack-encoded value, as a boolean, 8-64 bit integer, or 32-64 bit float. *Varying size|0 byte representing the end of packet|

All packets are encoded using [Consistent Overhead Byte Stuffing](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing), meaning that the only byte with a value of 0 received will be the end of packet marker. 

[\*] Byte sizes for MessagePack-encoded data are defined in the MessagePack specification: https://github.com/msgpack/msgpack/blob/master/spec.md#type-system. In MessagePack, values are encoded using the minimal possible space. Low-value unsigned integers, for example, will be stored in a single byte. With this encoding, the minimum size of a packet is 6 bytes.

The maximum length of a valid packet is 16 bytes.

# External Integrations

You can integrate the Arduino SDK into your software in several ways: By using the TelemetryJet CLI, or by reading packets manually in your own project.

## No-Code Usage with the TelemetryJet CLI
The Arduino SDK can easily be connected to other data sources and destinations such as a CSV or SQLite file with the TelemetryJet CLI. The CLI is a powerful command-line tool for defining, monitoring, and controlling embedded telemetry systems with minimal code. See the [TelemetryJet CLI Documentation](https://docs.telemetryjet.com/cli/) for more details.

The CLI has a builtin implementation of the Arduino SDK (the `telemetryjet-arduino-sdk` data source type), and handles transmitting and receiving data points with the Arduino SDK over a serial connection. 

## Reading Packets Manually
To read a packet manually in your own program, follow the following steps:
1. Store bytes in a fixed-size buffer until a 0 byte is received, indicating the end of a packet. _If the packet length limit (16 bytes) is exceeded, the data in the buffer is invalid and must be discarded._
2. When a 0 byte is received, run the received bytes through a COBS decoding algorithm, which will restore any 0 values in the packet.
3. Compute the checksum of the resulting packet. The sum of all bytes should equal 0xFF (255). If the checksum is any other value, discard the packet.
4. Skip over the first two bytes, for the checksum and padding/flag byte.
5. Read the MessagePack data:
   - Dimension ID: 16-bit unsigned integer, numerical identifier for the data point
   - Value Type: 8-bit unsigned integer, numeric identifier for the uncompressed value. Corresponds to an ID from the `DataPointType` enum, detailed in the [Value Types](#value-types) section above.
   - Value: 1-9 bytes, value of the data point, encoded as a MessagePack-compressed boolean, integer, or float.

This SDK implements an encoder and decoder in C++ in `TelemetryJet::update`, which you can copy and use in your projects.

# Resources & Notes
### Documentation
Full documentation for the TelemetryJet Arduino SDK is provided on the [TelemetryJet Documentation Site](https://docs.telemetryjet.com/arduino_sdk/).

### Compatibility

The TelemetryJet Arduino SDK requires Arduino IDE 1.5.x+.

# License
## MIT LICENSE

Copyright 2020 Chris Dalke

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
