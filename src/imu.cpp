#include <Arduino.h>
#include <Arduino_LSM6DS3.h>
#include <array>
#include "imu.h"

float Gx, Gy, Gz, Ax, Ay, Az;
float GyroOffsetX, GyroOffsetY, GyroOffsetZ;
bool ImuStatus;
bool GyroStatus;

static bool GyroCalibrate();

void ImuInit() {
    if (IMU.begin()) {
        if (!GyroCalibrate()) {
            Serial.println("Unable to calibrate Gyroscope; Attempting to proceed without Gyroscope.");
            GyroStatus = false;
        } else {
            GyroStatus = true;
        }
        ImuStatus = true;
        return;
    } else {
        Serial.println("Unable to begin IMU; Attempting to proceed without IMU.");
        ImuStatus = false;
        return;
    }
}

std::array<float, 8> ImuDataUpdate() {
    std::array<float, 8> imuData;
    imuData.fill(0);

    if (IMU.gyroscopeAvailable()) {
        if (IMU.readGyroscope(Gx, Gy, Gz)) {
            Gx += GyroOffsetX;
            Gy += GyroOffsetY;
            Gz += GyroOffsetZ;
            imuData[0] = 1.0f;
            imuData[1] = Gx;
            imuData[2] = Gy;
            imuData[3] = Gz;
        } else {
            std::fill_n(imuData.begin(), 4, 0.0f);
        }
    }

    if (IMU.accelerationAvailable()) {
        if (IMU.readAcceleration(Ax, Ay, Az)) {
            imuData[4] = 1.0f;
            imuData[5] = Ax;
            imuData[6] = Ay;
            imuData[7] = Az;
        } else {
            std::fill_n(imuData.begin() + 4, 4, 0.0f);
        }
    }

    return imuData;
}

bool ImuCheckFall() {
    std::array<float, 8> imuData = ImuDataUpdate();
    bool gyroDataAvailable = imuData[0] != 0.0f;
    bool accelDataAvailable = imuData[4] != 0.0f;

    if (gyroDataAvailable && accelDataAvailable) {
        if ((Gx > 100 || Gx < -100) && (Ax > 100 || Ax < -100)) return true;
        if ((Gy > 100 || Gy < -100) && (Ay > 100 || Ay < -100)) return true;
        if ((Gz > 100 || Gz < -100) && (Az > 100 || Az < -100)) return true;
        return false;
    }

    if (gyroDataAvailable) {
        if (Gx > 100 || Gx < -100) return true;
        if (Gy > 100 || Gy < -100) return true;
        if (Gz > 100 || Gz < -100) return true;
        return false;
    }

    if (accelDataAvailable) {
        if (Ax > 100 || Ax < -100) return true;
        if (Ay > 100 || Ay < -100) return true;
        if (Az > 100 || Az < -100) return true;
        return false;
    }

    return false;
}

static bool GyroCalibrate() {
    unsigned int Samples = 500;
    unsigned int Errors = 0;
    Gx = Gy = Gz = 0;
    GyroOffsetX = GyroOffsetY = GyroOffsetZ = 0;

    for (unsigned int i = 0; i < Samples; i++) {
        if (IMU.gyroscopeAvailable()) {
            if (!IMU.readGyroscope(Gx, Gy, Gz)) {
                #ifdef DEBUG
                Serial.println("Failed to read gyroscope data!");
                #endif
                Errors++;
                continue;
            }
        }
        GyroOffsetX += Gx;
        GyroOffsetY += Gy;
        GyroOffsetZ += Gz;
    }

    if (Errors < Samples) {
        GyroOffsetX /= (Samples - Errors);
        GyroOffsetY /= (Samples - Errors);
        GyroOffsetZ /= (Samples - Errors);
    } else {
        Serial.println("Gyro Calibration Failed; No Data Points Available!");
        return false;
    }

    Serial.print("Gyroscope Calibration Completed with ");
    Serial.print(Samples - Errors);
    Serial.print(" data points out of ");
    Serial.print(Samples);
    Serial.println(" attempted samples");
    return true;
}