#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <U8g2lib.h>

void oledInit();
void oledShowStartup(const char* msg);
void oledUpdateChannels(int16_t roll, int16_t pitch, int16_t throttle, int16_t yaw);

#endif