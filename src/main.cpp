#include <Arduino.h>
#include "sensors.h"
#include "display.h"
#include "communication.h"
#include "stateMachine.h"
#include "flashlight.h"

const unsigned long SEND_INTERVAL = 2000;

unsigned long lastSendMillis = 0;
unsigned long lastCheckInMillis = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("serial started");

  sensorsInit();
  Serial.println("sensorsInit done");

  displayInit();
  Serial.println("displayInit done");

  WifiInit();
  Serial.println("WifiInit done");

  StateMachineInit();
  Serial.println("StateMachineInit done");

  FlashlightInit();
  Serial.println("FlashlightInit done");
}

void loop() {
  unsigned long currentMillis = millis();
  
  sensorsUpdate();
  StateMachineUpdate();
  FlashlightUpdate();
  displayUpdate(currentMillis, (int)temperature, (int)humidity, 0);

  if (currentMillis - lastSendMillis >= SEND_INTERVAL) {
    unsigned long timeSinceLastCheckIn = currentMillis - getLastResolvedMillis();
    SendPacket(SensorDataPackager(), DEVICE_ID, timeSinceLastCheckIn, StateMachineGetStatus());
    lastSendMillis = currentMillis;
  }
}