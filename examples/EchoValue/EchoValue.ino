#include <TelemetryJet.h>

// This example echos a value that it receives on the "in"
// Dimension as a value on the "out" dimension.

TelemetryJet telemetry(&Serial, 0);

Dimension* inValue;
Dimension* outValue;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  telemetry.begin();
  inValue = telemetry.createDimension("in");
  outValue = telemetry.createDimension("out");
}

void loop() {
  telemetry.update();

  outValue->set(inValue->get());

  delay(100);
}
