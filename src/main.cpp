#include <Arduino.h>
#include "sensors.h"
#include "display.h"

void setup() {
  Serial.begin(9600);
  sensorsInit();
  displayInit();
}

void loop() {
  sensorsUpdate();
  displayUpdate(millis(), (int)temperature, (int)humidity, 0);
}