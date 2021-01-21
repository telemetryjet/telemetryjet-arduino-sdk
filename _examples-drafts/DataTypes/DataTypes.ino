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

void setup() {
  // Initialize the serial stream.
  Serial.begin(115200);
  while (!Serial);
}

void loop() {
  // Update the telemetry stream
  telemetry.update();
  
}
