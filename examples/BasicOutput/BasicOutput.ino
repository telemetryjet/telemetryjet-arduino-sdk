/*
TelemetryJet Arduino SDK
Chris Dalke <chrisdalke@gmail.com>
https://github.com/telemetryjet/telemetryjet-arduino-sdk

Lightweight communication library for hardware telemetry data. 
Handles bidirectional communication and state management for data points. 
-------------------------------------------------------------------------
Part of the TelemetryJet platform -- Collect, analyze, and share
data from your hardware. Code not required.

Distributed "as is" under the MIT License. See LICENSE.md for details.
*/

#include <TelemetryJet.h>

// Initialize an instance of the TelemetryJet SDK.
// The first parameter is the stream to communicate over.
// The second parameter is the interval, in milliseconds, to check and send data.
// Here, the SDK is configured to send new data every 100ms.
TelemetryJet telemetry(&Serial, 100);

// Create two dimensions.
// Each dimension is created with an integer "key",
// which uniquely identifies it.
Dimension testValue1 = telemetry.createDimension(1);
Dimension testValue2 = telemetry.createDimension(2);

void setup() {
  // Send data in "delta" mode, meaning the data points are only transmitted if they change.
  telemetry.setDeltaMode(true);
  
  // Initialize the serial stream.
  // TelemetryJet is transport-agnostic, so you are responsible
  // for setting up the connection, including serial baud rate.
  Serial.begin(115200);
  while (!Serial);
}

void loop() {
  // Process new data if available, and periodically send data points.
  // If possible, avoid blocking logic in your main loop, to allow the instance
  // time to process serial data as it is received.
  telemetry.update();
  
  // Update the dimensions with an oscillating signal.
  // Note that although this loop is running as fast as possible,
  // TelemetryJet will only send data at the interval specified earlier.
  testValue1.setFloat32(sin(millis() / 1000.0) * 100.0);
  testValue2.setFloat32(sin(millis() / 2000.0) * 200.0);
}
