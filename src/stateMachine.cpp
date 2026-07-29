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
const unsigned long ALERT_STATE_DURATION = 3000UL;
unsigned long CHECK_IN_INTERVAL = 900000UL;

unsigned long panicStart = 0;
unsigned long supCallStart = 0;

bool emergencyDialerTriggered = false;
unsigned long lastResolvedMillis = 0;

static int prevBtnOk = HIGH;
static int prevBtnSup = HIGH;
static int prevBtn911 = HIGH;

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

  int btnOk = digitalRead(PIN_BTN_OK);
  int btnSup = digitalRead(PIN_BTN_SUP);
  int btn911 = digitalRead(PIN_BTN_911);

  if (btn911 == LOW && prevBtn911 == HIGH) {
    digitalWrite(PIN_ALERT_BUZZ, LOW);
    currState = ST_PANIC;
    panicStart = now;
    Serial.println("AT+ATD911;");
    Serial.println("Sending logs and sensor telemetry to supervisor...");
  } else if (btnSup == LOW && prevBtnSup == HIGH) {
    digitalWrite(PIN_ALERT_BUZZ, LOW);
    currState = ST_SUP_CALL;
    supCallStart = now;
    Serial.println("AT+ATD;");
  }

  switch (currState) {
    case ST_CHECKIN:
      if (now - checkinStart >= TIMEOUT_5MIN) {
        unresponsiveStart = now;
        emergencyDialerTriggered = false;
        currState = ST_UNRESPONSIVE;
        break;
      }

      if (btnOk == LOW && prevBtnOk == HIGH) {
        resolveToActive();
      }
      break;

    case ST_SUP_CALL:
      if (btnOk == LOW && prevBtnOk == HIGH) {
        resolveToActive();
      } else if (now - supCallStart >= ALERT_STATE_DURATION) {
        resolveToActive();
      }
      break;

    case ST_PANIC:
      if (btnOk == LOW && prevBtnOk == HIGH) {
        resolveToActive();
      } else if (now - panicStart >= ALERT_STATE_DURATION) {
        resolveToActive();
      }
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

  prevBtn911 = btn911;
  prevBtnSup = btnSup;
  prevBtnOk = btnOk;
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

void setCheckInInterval(unsigned long val) {
  CHECK_IN_INTERVAL = val;
}

unsigned long getLastResolvedMillis() {
  return lastResolvedMillis;
}