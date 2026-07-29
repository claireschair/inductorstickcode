#ifndef DISPLAY_H
#define DISPLAY_H

void displayInit();
void displayUpdate(unsigned long currentMillis, int temp, int humidity, int sound);
void setAnnouncement(String message);
void setEmployeeName(String name);

#endif