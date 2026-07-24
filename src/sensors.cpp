#include <Arduino.h>
#include "sensors.h"
#include "imu.h"

const int dhtPin = 8;
const int dhtType = DHT11;

DHT dht(dhtPin, dhtType);

const int tempMaxThreshold = 25;
const int tempMinThreshold = 15;
const int humidityThreshold = 60;

std::array<float, 8> ImuDataPackage;

unsigned long lastDHTRead = 0;
float temperature = 0;
float humidity = 0;

unsigned long lastDebugPrint = 0;
const int debugInterval = 1000;

bool noiseDetected = false;
bool fallDetected = false;

bool checkIn = false;

bool tempHighAlertActive = false;
bool tempLowAlertActive = false;
bool humidityAlertActive = false;
bool noiseAlertActive = false;
bool fallAlertActive = false;

bool checkAlert(bool condition, bool &alertActive, const char* message) {
  bool triggered = false;
  if (condition) {
    if (!alertActive) {
      Serial.println(message);
      triggered = true;
    }
    alertActive = true;
  } else {
    alertActive = false;
  }
  return triggered;
}

void sensorsInit() {
  dht.begin();
}

std::array<float, 10> SensorDataPackager(){
  std::array<float, 10> dataPacket;
  std::copy(ImuDataPackage.begin(), ImuDataPackage.end(), dataPacket.begin());
  dataPacket[8] = temperature;
  dataPacket[9] = humidity;
  return dataPacket;
}

void sensorsUpdate() {
  if (millis() - lastDHTRead > 2000) {
    float newTemp = dht.readTemperature();
    float newHumidity = dht.readHumidity();

    if (!isnan(newTemp)) {
      temperature = newTemp;
    }

    if (!isnan(newHumidity)) {
      humidity = newHumidity;
    }

    lastDHTRead = millis();
  }

  #ifdef DEBUG
  if (millis() - lastDebugPrint > debugInterval) {
    Serial.print("Temperature: ");
    Serial.println(temperature);

    Serial.print("Humidity: ");
    Serial.println(humidity);

    Serial.print("Noise status: ");
    Serial.println(noiseDetected);

    Serial.print("Fall status: ");
    Serial.println(fallDetected);

    lastDebugPrint = millis();
  }
  #endif

  if (checkAlert(temperature > tempMaxThreshold, tempHighAlertActive, "Temperature above max threshold")) checkIn = true;
  if (checkAlert(temperature < tempMinThreshold, tempLowAlertActive, "Temperature below min threshold")) checkIn = true;
  if (checkAlert(humidity > humidityThreshold, humidityAlertActive, "Humidity above threshold")) checkIn = true;
  if (checkAlert(noiseDetected, noiseAlertActive, "Noise detected")) checkIn = true;
  if (checkAlert(fallDetected, fallAlertActive, "Fall detected")) checkIn = true;

  if (checkIn) {
    Serial.println("are you okay?");
    checkIn = false;
  }
}