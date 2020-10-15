#include <TelemetryJet.h>

// Create a TelemetryJet instance,
// communicating on the Serial stream
// and sending data once every 1000ms.
// Setting this to 0 will make the telemetry
// send values right away!
TelemetryJet telemetry(&Serial, 100);

Dimension timestamp;
Dimension yValue1;
Dimension yValue2;

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

  // Create a data point in the cache.
  // This returns an integer ID which can be used to set values.
  //timestamp = telemetry.createDimension("timestamp");
  yValue1 = telemetry.createDimension("yValue1");
  yValue2 = telemetry.createDimension("yValue2");
}

void loop() {
  // Processes new data if available, and periodically sends data points.
  telemetry.update();

  // Create an oscillating signal from the timer
  float testValue1 = sin(millis() / 1000.0) * 100.0;
  float testValue2 = sin(millis() / 750.0) * 200.0 + 25;

  // Update the data point.
  // Note that although this loop is running as fast as possible,
  // TelemetryJet will only send data at the interval specified earlier.
  //telemetry.set(timestamp, millis());
  telemetry.set(yValue1, testValue1);
  telemetry.set(yValue2, testValue2);
}
