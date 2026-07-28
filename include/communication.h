#pragma once

#include <array>
#include <Arduino.h>

void WifiInit();
void EnteredZone(String newZone);
void StartCheckIn();
void EndCheckIn();
void SendPacket(const std::array<float, 10>& sensorPacket, uint8_t id, unsigned long timeSinceLastCheckIn, String status);