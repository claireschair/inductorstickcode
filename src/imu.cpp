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

    if (GyroStatus && IMU.gyroscopeAvailable()) {
        float rawGx, rawGy, rawGz;
        if (IMU.readGyroscope(rawGx, rawGy, rawGz)) {
            Gx = rawGx - GyroOffsetX;
            Gy = rawGy - GyroOffsetY;
            Gz = rawGz - GyroOffsetZ;
        }
    }

    if (GyroStatus) {
        imuData[0] = 1.0f;
        imuData[1] = Gx;
        imuData[2] = Gy;
        imuData[3] = Gz;
    }

    if (ImuStatus && IMU.accelerationAvailable()) {
        float rawAx, rawAy, rawAz;
        if (IMU.readAcceleration(rawAx, rawAy, rawAz)) {
            Ax = rawAx;
            Ay = rawAy;
            Az = rawAz;
        }
    }

    if (ImuStatus) {
        imuData[4] = 1.0f;
        imuData[5] = Ax;
        imuData[6] = Ay;
        imuData[7] = Az;
    }

    return imuData;
}

bool ImuCheckFall(const std::array<float, 8>& imuData) {
    bool gyroDataAvailable = imuData[0] != 0.0f;
    bool accelDataAvailable = imuData[4] != 0.0f;

    const float GYRO_THRESH = 675.0f;   // deg/s
    const float ACCEL_THRESH = 2.3f;    // g

    float gyroX = imuData[1];
    float gyroY = imuData[2];
    float gyroZ = imuData[3];

    float accelX = imuData[5];
    float accelY = imuData[6];
    float accelZ = imuData[7];

    if (gyroDataAvailable && accelDataAvailable) {
        float gyroMag = sqrt(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);
        float accelMag = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

        return (gyroMag > GYRO_THRESH &&
                accelMag > ACCEL_THRESH);
    }

    if (gyroDataAvailable) {
        float gyroMag = sqrt(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);
        return gyroMag > GYRO_THRESH;
    }

    if (accelDataAvailable) {
        float accelMag = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
        return accelMag > ACCEL_THRESH;
    }

    return false;
}

static bool GyroCalibrate() {
    unsigned int Samples = 500;
    unsigned int collected = 0;
    Gx = Gy = Gz = 0;
    GyroOffsetX = GyroOffsetY = GyroOffsetZ = 0;

    unsigned long startMs = millis();
    while (collected < Samples && millis() - startMs < 5000) {
        if (IMU.gyroscopeAvailable() && IMU.readGyroscope(Gx, Gy, Gz)) {
            GyroOffsetX += Gx;
            GyroOffsetY += Gy;
            GyroOffsetZ += Gz;
            collected++;
        }
        delay(2);
    }

    if (collected > 0) {
        GyroOffsetX /= collected;
        GyroOffsetY /= collected;
        GyroOffsetZ /= collected;
    } else {
        Serial.println("Gyro Calibration Failed; No Data Points Available!");
        return false;
    }

    Serial.print("Gyroscope Calibration Completed with ");
    Serial.print(collected);
    Serial.println(" data points");
    return true;
}