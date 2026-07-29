#include <Arduino.h>
#include "display.h"
#include "stateMachine.h"
#include <LiquidCrystal.h>
#include <string>

const int RS = 7, EN = 6, D4 = 5, D5 = 4, D6 = 3, D7 = 2;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

const int SCROLL_DELAY = 500;
const int FLASH_DELAY = 500;
const int SENSOR_REFRESH = 1000;
const int FLASH_COUNT = 6;
const int SCROLL_REPEAT = 2;
const int SCROLL_GAP = 5;

enum DisplayState {
  IDLE,
  FLASHING,
  SCROLLING
};
DisplayState currentState = IDLE;

unsigned long previousStateMillis = 0;
unsigned long previousNameMillis = 0;
unsigned long previousSensorMillis = 0;

int flashCount = 0;
bool isFlashVisible = false;
int scrollIndex = 0;
int scrollLoops = 0;
int nameScrollIndex = 0;

String announcementMsg = "";

void triggerAnnouncement(unsigned long currentMillis);
void handleAnnouncement(String message, unsigned long currentMillis);
void displayName(String name, unsigned long currentMillis);
void scrollText(String text, int startIndex, int textLen, int loopLen);
void displaySensors(int temp, int humidity, int sound, unsigned long currentMillis);

void displayInit() {
  lcd.begin(16, 2);
}

void setAnnouncement(String message) {
  if (message.length() > 0) {
    announcementMsg = message;
    triggerAnnouncement(millis());
  }
}

void displayUpdate(unsigned long currentMillis, int temp, int humidity, int sound) {
  if (StateMachineGetStatus() == "checkin") {
    lcd.setCursor(0, 0);
    lcd.print("Are you OK?     ");
  } else {
    if (announcementMsg.length() > 0) {
      handleAnnouncement(announcementMsg, currentMillis);
    }
    displayName("A VERY LONG NAME THAT SLIDES", currentMillis);
  }

  displaySensors(temp, humidity, sound, currentMillis);
}

void triggerAnnouncement(unsigned long currentMillis) {
  if (currentState == IDLE) {
    currentState = FLASHING;
    flashCount = 0;
    isFlashVisible = true;
    scrollLoops = 0;
    scrollIndex = 0;
    previousStateMillis = currentMillis;
  }
}

void handleAnnouncement(String message, unsigned long currentMillis) {
  if (currentState == IDLE) return;

  if (currentState == FLASHING) {
    if (currentMillis - previousStateMillis >= FLASH_DELAY) {
      previousStateMillis = currentMillis;

      lcd.setCursor(0, 0);
      lcd.print(isFlashVisible ? "  ANNOUNCEMENT  " : "                ");

      isFlashVisible = !isFlashVisible;
      flashCount++;

      if (flashCount >= FLASH_COUNT) {
        currentState = SCROLLING;
        scrollIndex = 0;
        previousStateMillis = currentMillis;
      }
    }
  } else if (currentState == SCROLLING) {
    int len = message.length();
    int totalLoopLength = len + SCROLL_GAP;

    if (currentMillis - previousStateMillis >= SCROLL_DELAY) {
      previousStateMillis = currentMillis;

      scrollText(message, scrollIndex, len, totalLoopLength);
      scrollIndex++;

      if (scrollIndex >= totalLoopLength) {
        scrollIndex = 0;
        scrollLoops++;

        if (scrollLoops >= SCROLL_REPEAT) {
          currentState = IDLE;
          announcementMsg = "";
          nameScrollIndex = 0;
          previousNameMillis = currentMillis;

          lcd.setCursor(0, 0);
          lcd.print("                ");
        }
      }
    }
  }
}

void displayName(String name, unsigned long currentMillis) {
  if (currentState != IDLE) return;

  int len = name.length();

  if (len <= 16) {
    lcd.setCursor(0, 0);
    lcd.print(name);
    for (int i = len; i < 16; i++) {
      lcd.print(' ');
    }
    return;
  }

  int totalLoopLength = len + SCROLL_GAP;

  if (currentMillis - previousNameMillis >= SCROLL_DELAY) {
    previousNameMillis = currentMillis;

    scrollText(name, nameScrollIndex, len, totalLoopLength);

    nameScrollIndex++;
    if (nameScrollIndex >= totalLoopLength) {
      nameScrollIndex = 0;
    }
  }
}

void scrollText(String text, int startIndex, int textLen, int loopLen) {
  lcd.setCursor(0, 0);
  for (int i = 0; i < 16; i++) {
    int rawIndex = startIndex + i;
    int charIndex = (rawIndex % loopLen + loopLen) % loopLen;

    if (charIndex < textLen) {
      lcd.print(text[charIndex]);
    } else {
      lcd.print(' ');
    }
  }
}

void displaySensors(int temp, int humidity, int sound, unsigned long currentMillis) {
  if (currentMillis - previousSensorMillis >= SENSOR_REFRESH) {
    previousSensorMillis = currentMillis;

    lcd.setCursor(0, 1);

    lcd.print("T:");
    lcd.print(temp);
    lcd.print("C H:");

    lcd.print(humidity);
    lcd.print(" S:");

    lcd.print(sound);

    if (sound < 10) {
      lcd.print("  ");
    } else if (sound < 100) {
      lcd.print(" ");
    }
  }
}