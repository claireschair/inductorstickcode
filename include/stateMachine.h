#pragma once

#include <Arduino.h>

void StateMachineInit();
void StateMachineUpdate();
void StateMachineTriggerCheckIn();
String StateMachineGetStatus();
void setCheckInInterval(unsigned long val);
unsigned long getLastResolvedMillis();