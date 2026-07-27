#pragma once

#include <array>
#include <string>

void WifiInit();
void EnteredZone(string newZone);
void StartCheckIn();
void EndCheckIn();
void SendPacket(std::array<float, 10> sensorPacket, uint8_t id, unsigned long timeSinceLastCheckIn, String status);