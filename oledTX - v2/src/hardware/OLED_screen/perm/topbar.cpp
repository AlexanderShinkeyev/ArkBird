#include "topbar.h"
#include <Arduino.h>

#define TOPBAR_HEIGHT 12
#define VBAT_PIN GPIO_NUM_1
#define VDIV_FACTOR 1.68f

static void drawPageIndicator(U8G2* u8g2, uint8_t page, uint8_t numPages) {
    for (uint8_t i = 0; i < numPages; i++) {
        uint8_t x = 2 + i * 7;
        if (i == page) u8g2->drawBox(x, 2, 4, 4);
        else           u8g2->drawFrame(x, 2, 4, 4);
    }
}

static void drawButtonDots(U8G2* u8g2, bool btn33, bool btn47, bool btn39, bool btn40) {
    uint8_t btnY = 4;
    btn33 ? u8g2->drawDisc(50, btnY, 3) : u8g2->drawCircle(50, btnY, 3);
    btn47 ? u8g2->drawDisc(60, btnY, 3) : u8g2->drawCircle(60, btnY, 3);
    btn39 ? u8g2->drawDisc(70, btnY, 3) : u8g2->drawCircle(70, btnY, 3);
    btn40 ? u8g2->drawDisc(80, btnY, 3) : u8g2->drawCircle(80, btnY, 3);
}

void topbarInitBattery() {
    pinMode(VBAT_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(VBAT_PIN, ADC_11db);
}

float topbarReadVoltage() {
    uint16_t raw = analogRead(VBAT_PIN);
    return (raw / 4095.0f) * 3.3f * VDIV_FACTOR;
}

uint16_t topbarReadRaw() {
    return analogRead(VBAT_PIN);
}

void topbarRender(U8G2* u8g2, uint8_t page, uint8_t numPages, uint16_t fps,
                  bool btn33, bool btn47, bool btn39, bool btn40,
                  float voltage) {
    drawPageIndicator(u8g2, page, numPages);
    drawButtonDots(u8g2, btn33, btn47, btn39, btn40);
    
    // Voltage boxed
    if (voltage > 0.0f) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%.2fV", voltage);
        u8g2->setFont(u8g2_font_5x7_tr);
        u8g2->drawFrame(88, 0, 28, 10);
        u8g2->drawStr(90, 8, buf);
    }
    
    // FPS top-right
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", fps);
    u8g2->setFont(u8g2_font_5x7_tr);
    u8g2->drawStr(118, 8, buf);
}

uint8_t topbarGetOffset() {
    return TOPBAR_HEIGHT;
}