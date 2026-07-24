#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "imu.h"

const char* SSID = "AINSELY_WIFI_SSID";
const char* Password = "AINSELY_WIFI_PASSWORD";

const char* ServerIP = "Website IP";
const int port = 8080;

unsigned long PreviousMillis = 0;

void WifiInit(){    
    unsigned long currentMillis = 0;
    while (WiFi.begin(ssid, password) != WL_CONNECTED) {
        unsigned long currentMillis = millis();

        if((currentMillis - PreviousMillis) > 100){
            Serial.println("Connecting to Wi-Fi...");
            currentMillis = millis();
        }
    }
    Serial.println("Connected to Wi-Fi!");
    Serial.print("Arduino IP: ");
    Serial.println(WiFi.localIP());
}

void SendPacket(std::array<float, 10> sensorPacket, uint8_t id, unsigned long timeSinceLastCheckIn){
    float Gx, Gy, Gz, Ax, Ay, Az;
    Gx = Gy = Gz = Ax = Ay = Az = -1;
    if(client.connect(ServerIP, port)){
        JsonDocument packet;
        packet["id"] = id;
        packet["status"] = "active";
        packet["timeSinceLastCheckIn"] = timeSinceLastCheckIn / 1000;
        JsonObject sensor = packet["sensorData"].to<JsonObject>();
        sensorData["temperature"] = sensorPacket[8];
        sensorData["humidity"] = sensorPacket[9];
        JsonObject gyroData = packet["gyroData"].to<JsonObject>();
        if(sensorPacket[0] != 0){
            gyroData["x"] = sensorPacket[1];
            gyroData["y"] = sensorPacket[2];
            gyroData["z"] = sensorPacket[3];
        }
        else{
            gyroData["x"] = nullptr;
            gyroData["y"] = nullptr;
            gyroData["z"] = nullptr;
        }
        JsonObject accelData = packet["accelData"].to<JsonObject>();
        if(sensorPacket[4] != 0){
            accelData["x"] = sensorPacket[5];
            accelData["y"] = sensorPacket[6];
            accelData["z"] = sensorPacket[7];
        }
        else{
            accelData["x"] = nullptr;
            accelData["y"] = nullptr;
            accelData["z"] = nullptr;
        }
    }
}