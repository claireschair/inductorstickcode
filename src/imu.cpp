#include <Arduino.h>
#include <Arduino_LSM6DS3.h>
#include <array> 
#include "imu.h"

float Gx, Gy, Gz, Ax, Ay, Az;
float GyroOffsetX, GyroOffsetY, GyroOffsetZ;
bool ImuStatus;
bool GyroStatus;


void ImuInit(){
    if(IMU.begin()){
        if(!GyroCalibrate()){
            Serial.println("Unable to calibrate Gyroscope; Attempting to procede without Gyroscope.");
            GyroStatus = false;
        }
        else{
            GyroStatus = true;
        }
        ImuStatus = true;
        return;
    }
    else{
        Serial.println("Unable to begin IMU; Attempting to procede without IMU.");
        ImuStatus = false;
        return;
    }
}

std::array<float, 8> ImuDataUpdate(){
    std::array<float, 8> imuData;
    imuData.fill(0);
    if(IMU.gyroscopeAvailable()){
        ImuData[0];
        if(IMU.readGyroscope(Gx, Gy, Gz)){
            Gx += GyroOffsetX;
            Gy += GyroOffsetY;
            Gz += GyroOffsetZ;
            imuData[0] = 1f;
            imuData[1] = Gx;
            imuData[2] = Gy;
            imuData[3] = Gz;
        }
        else{
            std::fill_n(imuData.begin(), 4, 0);
        }
    }
    if(IMU.accelerationAvailable()){
        if(IMU.readAcceleration(Ax, Ay, Az)){
            imuData[4] = 1f;
            imuData[5] = Ax;
            imuData[6] = Ay;
            imuData[7] = Az;
        }
        else{
            std::fill_n(imuData.begin() + 3, 4, 0);
        }
    }
    return imuData;
}

std::array<float, 8> ImuUpdate(){
    std::array<float, 8> imuData = ImuDataUpdate();
    bool gyroDataAvailable;
    bool accelDataAvailable;
    for (int i = 0; i < 8; i++){
        switch (i){
            case 0:
                if(imuData[0] = 0f){
                i += 3;
                }
                break;
            case 3:
                if(imuData[3] = 0f){
                i += 100;
                }
                break;
            default:
                Serial.println("Failed to parse imu data!");
                return false;
                break;
        }
    }
    if(gyroDataAvailable && accelDataAvailable){
        if((Gx > 100) && (Ax > 100)){
            return true;
        }
        else if((Gy > 100 || Gy < -100) && (Ay > 100 || Ay < -100)){
            return true;
        }
        else if((Gz > 100 || Gz < -100) && (Az > 100 || Az < -100)){
            return true;
        }
        else{
            return false;
        }
    }
    else if(gyroDataAvailable){
        if(Gx > 100){
            return true;
        }
        else if(Gy > 100 || Gy < -100){
            return true;
        }
        else if(Gz > 100 || Gz < -100){
            return true;
        }
        else{
            return false;
        }
    }
    else if(accelDataAvailable){
        if(Ax > 100){
            return true;
        }
        else if(Ay > 100 || Ay < -100){
            return true;
        }
        else if(Az > 100 || Az < -100){
            return true;
        }
        else{
            return false;
        }
    }
    if(!gyroDataAvailable && !accelDataAvailable){
        return false;
    }
}

static bool GyroCalibrate(){
    unsigned int Samples = 500;
    unsigned int Error;
    Gx = Gy = Gz = 0;
    GyroOffsetX = GyroOffsetY = GyroOffsetZ = Errors = 0;
    for(int i; i < Samples; i++){
        while(IMU.gyroscopeAvailable()){
            if(!IMU.readGyroscope(Gx, Gy, Gz)){

                #ifdef DEBUG
                Serial.println("Failed to read gyroscope data!");
                #endif

                Errors++;
                break;
            }
        }
        GyroOffsetX += Gx;
        GyroOffsetY += Gy;
        GyroOffsetZ += Gz;
    }
    if(Samples > Errors){
        GyroOffsetX /= (Samples - Errors);
        GyroOffsetY /= (Samples - Errors);
        GyroOffsetZ /= (Samples - Errors);
    }
    else if(Samples = Errors){
        Serial.println("Gyro Calibration Failed; No Data Points Available!");
        return false;
    }
    else if(
        Serial.println("Gyro Calibration Failed; Unexpected Error!");
        return false;
    )
    Serial.print("Gyroscope Calibration Completed with ");
    Serial.print(Samples - Errors);
    Serial.print(" data points out of ");
    Serial.print(Samples);
    Serial.println(" attempted samples");
    return true;
    // add temperature calibration maybe
}