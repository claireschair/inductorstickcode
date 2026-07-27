#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <string.h>
#include "imu.h"

const char* SSID = "AINSELY_WIFI_SSID";
const char* Password = "AINSELY_WIFI_PASSWORD";

const char* ServerIP = "Website IP";
const int port = 8080;

unsigned long PreviousWifiAttempt = 0;
unsigned long TimeSinceLastPaclet = 0;
String zone;
bool checkIn;

void WifiInit(){    
    unsigned long currentMillis = 0;
    while (WiFi.begin(ssid, password) != WL_CONNECTED) {
        unsigned long currentMillis = millis();

        if((currentMillis - PreviousWifiAttempt) > 100){
            Serial.println("Connecting to Wi-Fi...");
            currentMillis = millis();
        }
    }
    Serial.println("Connected to Wi-Fi!");
    Serial.print("Arduino IP: ");
    Serial.println(WiFi.localIP());
}

void EnteredZone(string newZone){
    zone = newZone;
}

void StartCheckIn(){
    checkIn = true;
}

void EndCheckIn(){
    checkIn = false;
}

void SendPacket(std::array<float, 10> sensorPacket, uint8_t id, unsigned long timeSinceLastCheckIn, String status){
    float Gx, Gy, Gz, Ax, Ay, Az;
    Gx = Gy = Gz = Ax = Ay = Az = -1;
    if(client.connect(ServerIP, port)){
        JsonDocument packet;
        packet["id"] = id;
        packet["zone"] = Zone;
        packet["status"] = status;
        packet["checkingIn"] = checkIn;
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

        serializeJson(packet, cilent);

        client.stop();

        Serial.println("POST request sent successfully via stream!");
    }
    else{
        Serial.println("Connection to server failed.");
    }
}