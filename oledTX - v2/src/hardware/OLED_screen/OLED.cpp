#include "hardware/OLED_screen/OLED.h"
#include "hardware/OLED_screen/perm/topbar.h"
#include <Wire.h>

#define VEXT GPIO_NUM_36

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 21, 18, 17);

static void drawPanel(uint8_t px, uint8_t py, const char* label, int16_t value, bool isThrottle) {
    u8g2.drawFrame(px, py, 42, 17);
    
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(px + 2, py + 7, label);
    
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", value);
    u8g2.setFont(u8g2_font_5x7_tn);
    u8g2.drawStr(px + 2, py + 15, buf);
    
    uint8_t barX = px + 30;
    uint8_t barY = py + 2;
    uint8_t barW = 10;
    uint8_t barH = 13;
    
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
    
    Wire1.begin(17, 18);
    Wire1.setClock(400000);
    
    u8g2.begin();
}

void oledShowStartup(const char* msg) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(4, 64, msg);
    u8g2.sendBuffer();
}

void oledUpdateChannels(int16_t roll, int16_t pitch, int16_t throttle, int16_t yaw) {
    uint8_t top = topbarGetOffset();
    u8g2.clearBuffer();
    drawPanel(0,  top,      "ROLL", roll, false);
    drawPanel(64, top,      "PTCH", pitch, false);
    drawPanel(0,  top + 26, "THRT", throttle, true);
    drawPanel(64, top + 26, "YAW",  yaw, false);
    u8g2.sendBuffer();
}

void oledUpdateTX(int16_t roll, int16_t pitch, int16_t throttle, int16_t yaw, 
                  int16_t axis39, int16_t axis41,
                  float throttleRaw, uint16_t fps, uint8_t page,
                  bool btn33, bool btn47, bool btn39, bool btn40,
                  float voltage) {
    u8g2.clearBuffer();
    
    uint8_t top = topbarGetOffset();
    topbarRender(&u8g2, page, 3, fps, btn33, btn47, btn39, btn40, voltage);
    
    drawPanel(0,  top,      "ROLL", roll, false);
    drawPanel(43, top,      "PTCH", pitch, false);
    drawPanel(86, top,      "J39",  axis39, false);
    drawPanel(0,  top + 17, "THRT", throttle, true);
    drawPanel(43, top + 17, "YAW",  yaw, false);
    drawPanel(86, top + 17, "J41",  axis41, false);
    
    if (throttleRaw > 0 && throttle == 0) {
        uint8_t rawH = (throttleRaw * 13) / 1000;
        u8g2.drawHLine(31, top + 17 + 15 - rawH, 3);
    }
    
    u8g2.sendBuffer();
}

void oledUpdateCalibration(int16_t rawRoll, int16_t rawPitch, int16_t rawYaw,
                           int16_t offsetRoll, int16_t offsetPitch, int16_t offsetYaw,
                           float voltage) {
    u8g2.clearBuffer();
    
    uint8_t top = topbarGetOffset();
    topbarRender(&u8g2, 2, 3, 0, false, false, false, false, voltage);
    
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(52, 8, "CALIBRATE");
    
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(2, top + 8, "RAW");
    
    char buf[16];
    snprintf(buf, sizeof(buf), "R:%4d", rawRoll);
    u8g2.drawStr(2, top + 20, buf);
    snprintf(buf, sizeof(buf), "P:%4d", rawPitch);
    u8g2.drawStr(2, top + 32, buf);
    snprintf(buf, sizeof(buf), "Y:%4d", rawYaw);
    u8g2.drawStr(2, top + 44, buf);
    
    u8g2.drawStr(45, top + 20, "Center");
    u8g2.drawStr(45, top + 32, "sticks");
    u8g2.drawStr(45, top + 44, "BTN2");
    
    u8g2.drawStr(88, top + 8, "OFF");
    snprintf(buf, sizeof(buf), "R:%4d", offsetRoll);
    u8g2.drawStr(88, top + 20, buf);
    snprintf(buf, sizeof(buf), "P:%4d", offsetPitch);
    u8g2.drawStr(88, top + 32, buf);
    snprintf(buf, sizeof(buf), "Y:%4d", offsetYaw);
    u8g2.drawStr(88, top + 44, buf);
    
    u8g2.sendBuffer();
}

void oledUpdateVoltage(float voltage, uint16_t raw, uint8_t page) {
    u8g2.clearBuffer();
    
    uint8_t top = topbarGetOffset();
    topbarRender(&u8g2, page, 3, 0, false, false, false, false, voltage);
    
    // Big voltage display centered
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2fV", voltage);
    u8g2.setFont(u8g2_font_logisoso24_tn);
    
    uint8_t w = u8g2.getStrWidth(buf);
    u8g2.drawStr((128 - w) / 2, top + 32, buf);
    
    // Raw ADC value
    snprintf(buf, sizeof(buf), "RAW: %d", raw);
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(36, top + 48, buf);
    
    u8g2.sendBuffer();
}