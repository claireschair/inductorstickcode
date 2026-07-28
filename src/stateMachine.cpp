#include <Arduino.h>
#include "stateMachine.h"
#include "communication.h"

#define PIN_BTN_OK 11
#define PIN_BTN_SUP 10
#define PIN_BTN_911 9
#define PIN_ALERT_BUZZ A2

#define ST_ACTIVE 0
#define ST_CHECKIN 1
#define ST_SUP_CALL 2
#define ST_PANIC 3
#define ST_UNRESPONSIVE 4

int currState = ST_ACTIVE;
unsigned long checkinStart = 0;
unsigned long unresponsiveStart = 0;
unsigned long lastActivePrint = 0;
const unsigned long ACTIVE_PRINT_INTERVAL = 1000;

const unsigned long TIMEOUT_5MIN = 300000UL;
const unsigned long WAIT_30MIN = 1800000UL;
const unsigned long CHECK_IN_INTERVAL = 900000UL;

bool emergencyDialerTriggered = false;
unsigned long lastResolvedMillis = 0;

void resolveToActive() {
  currState = ST_ACTIVE;
  EndCheckIn();
  lastResolvedMillis = millis();
}

void StateMachineInit() {
  pinMode(PIN_BTN_OK, INPUT_PULLUP);
  pinMode(PIN_BTN_SUP, INPUT_PULLUP);
  pinMode(PIN_BTN_911, INPUT_PULLUP);
  pinMode(PIN_ALERT_BUZZ, OUTPUT);

  StateMachineTriggerCheckIn();
}

void StateMachineTriggerCheckIn() {
  currState = ST_CHECKIN;
  checkinStart = millis();
  Serial.println("LCD_WRITE: yo u good chat?");
  StartCheckIn();
}

void StateMachineUpdate() {
  unsigned long now = millis();

  if (digitalRead(PIN_BTN_911) == LOW) {
    digitalWrite(PIN_ALERT_BUZZ, LOW);
    currState = ST_PANIC;
  } else if (digitalRead(PIN_BTN_SUP) == LOW) {
    digitalWrite(PIN_ALERT_BUZZ, LOW);
    currState = ST_SUP_CALL;
  }

  switch (currState) {
    case ST_CHECKIN:
      if (now - checkinStart >= TIMEOUT_5MIN) {
        unresponsiveStart = now;
        emergencyDialerTriggered = false;
        currState = ST_UNRESPONSIVE;
        break;
      }

      if (digitalRead(PIN_BTN_OK) == LOW) {
        resolveToActive();
      }
      break;

    case ST_SUP_CALL:
      Serial.println("AT+ATD;");
      resolveToActive();
      break;

    case ST_PANIC:
      Serial.println("AT+ATD911;");
      Serial.println("Sending logs and sensor telemetry to supervisor...");
      resolveToActive();
      break;

    case ST_UNRESPONSIVE:
      analogWrite(PIN_ALERT_BUZZ, 128);

      if (!emergencyDialerTriggered) {
        Serial.println("AT+ATD911;");
        emergencyDialerTriggered = true;
      }

      if (now - unresponsiveStart >= WAIT_30MIN) {
        digitalWrite(PIN_ALERT_BUZZ, LOW);
        resolveToActive();
      }
      break;

    case ST_ACTIVE:
      if (now - lastActivePrint >= ACTIVE_PRINT_INTERVAL) {
        Serial.println("SYS_STATUS: Monitoring background sensors");
        lastActivePrint = now;
      }

      if (now - lastResolvedMillis >= CHECK_IN_INTERVAL) {
        StateMachineTriggerCheckIn();
      }
      break;
  }
}

String StateMachineGetStatus() {
  switch (currState) {
    case ST_ACTIVE: return "active";
    case ST_CHECKIN: return "checkin";
    case ST_SUP_CALL: return "supervisor_call";
    case ST_PANIC: return "panic";
    case ST_UNRESPONSIVE: return "unresponsive";
    default: return "unknown";
  }
}