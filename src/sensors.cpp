#define DEBUG
#include <DHT.h>

const int dhtPin = 8;
const int dhtType = DHT11;

DHT dht(dhtPin, dhtType);

const int tempThreshold = 25;
const int humidityThreshold = 60;

unsigned long lastDHTRead = 0;
float temperature = 0;
float humidity = 0;

unsigned long lastDebugPrint = 0;
const int debugInterval = 1000;

bool noiseDetected = false;
bool fallDetected = false;

bool checkIn = false;

bool tempAlertActive = false;
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

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
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

  if (checkAlert(temperature > tempThreshold, tempAlertActive, "Temperature above threshold")) checkIn = true;
  if (checkAlert(humidity > humidityThreshold, humidityAlertActive, "Humidity above threshold")) checkIn = true;
  if (checkAlert(noiseDetected, noiseAlertActive, "Noise detected")) checkIn = true;
  if (checkAlert(fallDetected, fallAlertActive, "Fall detected")) checkIn = true;

  if (checkIn) {
    //add check in stuff here
    Serial.println("are you okay?");
    checkIn = false;
  }
}