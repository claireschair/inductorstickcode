#pragma once

#include <array>
#include <Arduino.h>

extern const uint8_t DEVICE_ID;

void WifiInit();
void EnteredZone(String newZone);
void StartCheckIn();
void EndCheckIn();
void SendPacket(const std::array<float, 10>& sensorPacket, uint8_t id, unsigned long timeSinceLastCheckIn, String status);
void ReceiveServerResponse();