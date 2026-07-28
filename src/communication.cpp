#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <array>
#include "communication.h"
#include "imu.h"


const char* SSID = "AINSELY_WIFI_SSID";
const char* Password = "AINSELY_WIFI_PASSWORD";

const char* ServerIP = "Website IP";
const int port = 8080;

WiFiClient client;

unsigned long PreviousWifiAttempt = 0;
String zone;
static bool checkIn;

void WifiInit() {
    Serial.println("Connecting to Wi-Fi...");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 3) {
        WiFi.begin(SSID, Password);
        unsigned long start = millis();
        while (millis() - start < 3000) {
            if (WiFi.status() == WL_CONNECTED) break;
            delay(100);
        }
        attempts++;
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Retrying Wi-Fi connection...");
        }
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to Wi-Fi!");
        Serial.print("Arduino IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Wi-Fi connection pending. Continuing setup...");
    }
}

void EnteredZone(String newZone) {
    zone = newZone;
}

void StartCheckIn() {
    checkIn = true;
}

void EndCheckIn() {
    checkIn = false;
}

void SendPacket(const std::array<float, 10>& sensorPacket, uint8_t id, unsigned long timeSinceLastCheckIn, String status) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi not connected; unable to send packet.");
        return;
    }
    if (client.connect(ServerIP, port)) {
        JsonDocument packet;
        packet["id"] = id;
        packet["zone"] = zone;
        packet["status"] = status;
        packet["checkingIn"] = checkIn;
        packet["timeSinceLastCheckIn"] = timeSinceLastCheckIn / 1000;

        JsonObject sensor = packet["sensorData"].to<JsonObject>();
        sensor["temperature"] = sensorPacket[8];
        sensor["humidity"] = sensorPacket[9];

        JsonObject gyroData = packet["gyroData"].to<JsonObject>();
        if (sensorPacket[0] != 0) {
            gyroData["x"] = sensorPacket[1];
            gyroData["y"] = sensorPacket[2];
            gyroData["z"] = sensorPacket[3];
        } else {
            gyroData["x"] = nullptr;
            gyroData["y"] = nullptr;
            gyroData["z"] = nullptr;
        }

        JsonObject accelData = packet["accelData"].to<JsonObject>();
        if (sensorPacket[4] != 0) {
            accelData["x"] = sensorPacket[5];
            accelData["y"] = sensorPacket[6];
            accelData["z"] = sensorPacket[7];
        } else {
            accelData["x"] = nullptr;
            accelData["y"] = nullptr;
            accelData["z"] = nullptr;
        }

        JsonObject position = packet["position"].to<JsonObject>();
        position["x"] = nullptr;
        position["y"] = nullptr;

        size_t jsonSize = measureJson(packet);

        #ifdef DEBUG
        serializeJsonPretty(packet, Serial);
        #endif

        client.println("POST /api/data HTTP/1.1");
        client.print("Host: "); client.println(ServerIP);
        client.println("Content-Type: application/json");
        client.print("Content-Length: "); client.println(jsonSize);
        client.println("Connection: close");
        client.println();

        serializeJson(packet, client);

        client.stop();

        Serial.println("POST request sent successfully via stream!");
    } else {
        Serial.println("Connection to server failed.");
    }
}