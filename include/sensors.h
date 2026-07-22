#ifndef SENSORS_H
#define SENSORS_H

#include <DHT.h>

extern DHT dht;
extern float temperature;
extern float humidity;
extern bool noiseDetected;
extern bool fallDetected;

void sensorsInit();
void sensorsUpdate();

#endif