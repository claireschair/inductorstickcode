#include <Arduino.h>
#include "sensors.h"
#include "display.h"
#include "communication.h"
#include "stateMachine.h"
#include "flashlight.h"

const uint8_t DEVICE_ID = 1;
const unsigned long SEND_INTERVAL = 2000;

unsigned long lastSendMillis = 0;
unsigned long lastCheckInMillis = 0;

void setup() {
  Serial.begin(9600);
  sensorsInit();
  displayInit();
  WifiInit();
  StateMachineInit();
  FlashlightInit();
}

void loop() {
  unsigned long currentMillis = millis();

  sensorsUpdate();
  StateMachineUpdate();
  FlashlightUpdate();
  displayUpdate(currentMillis, (int)temperature, (int)humidity, 0);

  if (currentMillis - lastSendMillis >= SEND_INTERVAL) {
    lastSendMillis = currentMillis;
    unsigned long timeSinceLastCheckIn = currentMillis - lastCheckInMillis;
    SendPacket(SensorDataPackager(), DEVICE_ID, timeSinceLastCheckIn, StateMachineGetStatus());
  }
}