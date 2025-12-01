#include "OLED.h"

#define VEXT GPIO_NUM_36

static U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 18, 17, 21);

static void drawPanel(uint8_t px, uint8_t py, const char* label, int16_t value, bool isThrottle) {
    u8g2.drawFrame(px, py, 64, 32);
    
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(px + 3, py + 10, label);
    
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", value);
    u8g2.setFont(u8g2_font_7x14B_tn);
    u8g2.drawStr(px + 3, py + 26, buf);
    
    uint8_t barX = px + 45;
    uint8_t barW = 15;
    uint8_t barH = 24;
    uint8_t barY = py + 4;
    
    u8g2.drawFrame(barX, barY, barW, barH);
    
    uint8_t fillH;
    if (isThrottle) {
        fillH = (value * barH) / 1000;
    } else {
        fillH = ((value + 1000) * barH) / 2000;
    }
    if (fillH > barH) fillH = barH;
    
    u8g2.drawBox(barX + 1, barY + barH - fillH, barW - 2, fillH);
}

void oledInit() {
    pinMode(VEXT, OUTPUT);
    digitalWrite(VEXT, LOW);
    delay(100);
    u8g2.begin();
}

void oledShowStartup(const char* msg) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(20, 35, msg);
    u8g2.sendBuffer();
}

void oledUpdateChannels(int16_t roll, int16_t pitch, int16_t throttle, int16_t yaw) {
    u8g2.clearBuffer();
    drawPanel(0,  0,  "ROLL", roll, false);
    drawPanel(64, 0,  "PTCH", pitch, false);
    drawPanel(0,  32, "THRT", throttle, true);
    drawPanel(64, 32, "YAW",  yaw, false);
    u8g2.sendBuffer();
}