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
TelemetryJet telemetry(&Serial, 100);
Dimension testValue1 = telemetry.createDimension(1);
Dimension testValue2 = telemetry.createDimension(2);

void setup() {
  // Send data in "text mode", which can be useful for debugging in the Arduino IDE.
  // In text mode, all data points are sent as text in a list at the write interval.
  //
  // The output for this program will look something like:
  // 68.21 186.08
  // 60.57 189.51
  // 52.32 192.50
  // ...
  //
  // Notes:
  // - In text mode, values cannot be sent to the device.
  // - Text mode is highly inefficient for data transmission, and should only be used
  //   for debugging purposes!
  telemetry.setTextMode(true);

  // Initialize the serial stream.
  Serial.begin(115200);
  while (!Serial);
}

void loop() {
  // Process new data if available, and periodically send data points.
  telemetry.update();

  // Update the dimensions with an oscillating signal.
  testValue1.setFloat32(sin(millis() / 1000.0) * 100.0);
  testValue2.setFloat32(sin(millis() / 2000.0) * 200.0);
}
