#include <TelemetryJet.h>

// Create a TelemetryJet instance,
// communicating on the Serial stream
// and sending data once every 1000ms.
TelemetryJet telemetry(&Serial, 1000);

Dimension* testValue;

unsigned long memCheckTimer = 0;

void setup() {
  // Initialize the serial stream before starting
  // the TelemetryJet instance.
  // TelemetryJet is transport-agnostic, so you 
  // are responsible for setting the connection settings
  // on either side.
  Serial.begin(115200);
  while (!Serial);
  
  telemetry.begin();

  // Create a dimension.
  // This returns a pointer to a Dimension object,
  // which you can use to get and set values.
  testValue = telemetry.createDimension("testValue");
}

void loop() {
  // Processes new data if available, and periodically sends data points.
  telemetry.update();

  // Create an oscillating signal from the timer
  float testValue1 = sin(millis() / 1000.0) * 100.0;

  // Update the dimension.
  // Note that although this loop is running as fast as possible,
  // TelemetryJet will only send data at the interval specified earlier.
  testValue->set(testValue1);
}
