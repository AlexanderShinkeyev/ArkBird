#ifndef TOPBAR_H
#define TOPBAR_H

#include <U8g2lib.h>

void topbarRender(U8G2* u8g2, uint8_t page, uint8_t numPages, uint16_t fps,
                  bool btn33, bool btn47, bool btn39, bool btn40,
                  float voltage = 0.0f);

uint8_t topbarGetOffset();

void topbarInitBattery();
float topbarReadVoltage();
uint16_t topbarReadRaw();

#endif