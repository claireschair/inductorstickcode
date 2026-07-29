#include <Arduino.h>
#include "flashlight.h"

const int flashlightPin = 12;
const int potPin = A6;

void FlashlightInit() {
  pinMode(flashlightPin, OUTPUT);
}

void FlashlightUpdate() {
  int potValue = analogRead(potPin);
  int brightness = map(potValue, 0, 1023, 0, 255);
  analogWrite(flashlightPin, brightness);
}