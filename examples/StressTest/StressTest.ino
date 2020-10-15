#include <TelemetryJet.h>

TelemetryJet telemetry(&Serial, 0);

Dimension** values;
int numValues = 1;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  values = (Dimension**)malloc(sizeof(Dimension*)*numValues);
  for (int i = 0; i < numValues; i++) {
    char* valueLabel = (char*)malloc(sizeof(char)*8);
    sprintf(valueLabel, "%d", i);
    values[i] = telemetry.createDimension(valueLabel);
  }
  
  telemetry.begin();
}

void loop() {
  telemetry.update();

  for (int i = 0; i < numValues; i++) {
    values[i]->set(sin(millis() / 1000.0 + (i * 0.1)) * 100.0);
  }
}
