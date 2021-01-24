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

// Create two dimensions for an input and output.
// In this example, we'll echo the input value back out.
Dimension inputValue = telemetry.createDimension(1);
Dimension outputValue = telemetry.createDimension(2);

void setup() {
  // Initialize the serial stream.
  Serial.begin(115200);
  while (!Serial);
}

void loop() {
  // Update the telemetry stream
  telemetry.update();

  // If we received a new value on dimension 1, echo it back on dimension 2
  // We can also check to verify the type of the input value
  if (inputValue.hasNewValue() && inputValue.hasFloat32()) {
    outputValue.setFloat32(inputValue.getFloat32());
  }  
}
