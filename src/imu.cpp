#include <Arduino.h>
#include <Arduino_LSM6DS3.h>
#include <array>
#include "imu.h"

float Gx, Gy, Gz, Ax, Ay, Az;
float GyroOffsetX, GyroOffsetY, GyroOffsetZ;
bool ImuStatus;
bool GyroStatus;

std::array<float, 8> imuPackage;

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

static std::array<float, 8> ImuDataUpdate() {
    std::array<float, 8> imuData;
    imuData.fill(0);

    if (IMU.gyroscopeAvailable()) {
        if (IMU.readGyroscope(Gx, Gy, Gz)) {
            Gx -= GyroOffsetX;
            Gy -= GyroOffsetY;
            Gz -= GyroOffsetZ;
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
    imuPackage = ImuDataUpdate();

    bool gyroDataAvailable = imuPackage[0] != 0.0f;
    bool accelDataAvailable = imuPackage[4] != 0.0f;

    const float GYRO_THRESH = 675.0f;   // deg/s
    const float ACCEL_THRESH = 2.3f;    // g

    if (gyroDataAvailable && accelDataAvailable) {
        float gyroMag = sqrt(Gx * Gx + Gy * Gy + Gz * Gz);
        float accelMag = sqrt(Ax * Ax + Ay * Ay + Az * Az);

        return (gyroMag > GYRO_THRESH &&
                accelMag > ACCEL_THRESH);
    }
    else if (gyroDataAvailable) {
        float gyroMag = sqrt(Gx * Gx + Gy * Gy + Gz * Gz);
        return gyroMag > GYRO_THRESH;
    }
    else if (accelDataAvailable) {
        float accelMag = sqrt(Ax * Ax + Ay * Ay + Az * Az);
        return accelMag > ACCEL_THRESH;
    }
    else{
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
        if (IMU.gyroscopeAvailable() && IMU.readGyroscope(Gx, Gy, Gz)) {
            GyroOffsetX += Gx;
            GyroOffsetY += Gy;
            GyroOffsetZ += Gz;
        } else {
            Errors++;
        }
        delay(5);
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