#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <U8g2lib.h>

void oledInit();
void oledShowStartup(const char* msg);
void oledUpdateChannels(int16_t roll, int16_t pitch, int16_t throttle, int16_t yaw);
void oledUpdateTX(int16_t roll, int16_t pitch, int16_t throttle, int16_t yaw,
                  int16_t axis39, int16_t axis41,
                  float throttleRaw, uint16_t fps, uint8_t page,
                  bool btn33 = false, bool btn47 = false, bool btn39 = false, bool btn40 = false,
                  float voltage = 0.0f);
void oledUpdateCalibration(int16_t rawRoll, int16_t rawPitch, int16_t rawYaw,
                           int16_t offsetRoll, int16_t offsetPitch, int16_t offsetYaw,
                           float voltage = 0.0f);
void oledUpdateVoltage(float voltage, uint16_t raw, uint8_t page);

#endif