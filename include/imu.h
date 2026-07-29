#pragma once

#include <array>

extern float GyroOffsetX, GyroOffsetY, GyroOffsetZ;
extern float Gx, Gy, Gz, Ax, Ay, Az;
extern bool ImuStatus;
extern bool GyroStatus;

void ImuInit();
extern std::array<float, 8> imuPackage;
bool ImuCheckFall();
