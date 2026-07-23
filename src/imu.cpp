#include <Arduino.h>
#include <Arduino_LSM6DSOX.h>

float Gx, Gy, Gz;

float GyroOffsetX, GyroOffsetY, GyroOffsetZ;

void ImuInit(){
    if(IMU.begin()){
        GyroCalibrate();
    }
    else{
        Serial.println("Unable to begin IMU; Critical Error!");
    }
}

void GyroCalibrate(){
    unsigned int Samples = 500;
    unsigned int Error;
    Gx = Gy = Gz = 0;
    GyroOffsetX = GyroOffsetY = GyroOffsetZ = Errors = 0;
    for(int i; i < Samples; i++){
        while(IMU.gyroscopeAvailable()){
            if(!IMU.readGyroscope(Gx, Gy, Gz)){
                Serial.println("Failed to read gyroscope data!")
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
        Serial.println("Gyro Calibration Failed; No Data Points Available!")
        return;
    }
    else if(
        Serial.println("Gyro Calibration Failed; Unexpected Error!")
        return;
    )
    Serial.print("Gyroscope Calibration Completed with ");
    Serial.print(Samples - Errors);
    Serial.print(" data points out of ");
    Serial.print(Samples);
    Serial.println(" attempted samples");
}