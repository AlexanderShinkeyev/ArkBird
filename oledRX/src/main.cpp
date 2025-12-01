// RXB - PCA9685 servos, throttle is 0-1000, OLED display
#include <Arduino.h>
#include <RadioLib.h>
#include <Wire.h>
#include <U8g2lib.h>

SX1262 radio = new Module(8, 14, 12, 13);

// OLED on second I2C bus
#define VEXT GPIO_NUM_36
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 18, 17, 21);

// PCA9685 on primary I2C
#define SDA_PIN 4
#define SCL_PIN 5
#define PCA9685_ADDR 0x40

#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define PCA9685_LED0_ON_L 0x06

#define SERVO_MIN 205
#define SERVO_MID 307
#define SERVO_MAX 410

struct ControlPacket {
    int16_t channels[4];
    uint8_t sequence;
};

ControlPacket pkt;
volatile bool rxFlag = false;
unsigned long lastDisplayUpdate = 0;

void IRAM_ATTR onRx() { rxFlag = true; }

void pcaWrite(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(PCA9685_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void pcaInit() {
    pcaWrite(PCA9685_MODE1, 0x80);
    delay(10);
    pcaWrite(PCA9685_MODE1, 0x10);
    pcaWrite(PCA9685_PRESCALE, 121);
    pcaWrite(PCA9685_MODE1, 0x00);
    delay(5);
    pcaWrite(PCA9685_MODE1, 0x20);
}

void setServo(uint8_t ch, uint16_t ticks) {
    uint8_t reg = PCA9685_LED0_ON_L + (ch * 4);
    Wire.beginTransmission(PCA9685_ADDR);
    Wire.write(reg);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(ticks & 0xFF);
    Wire.write(ticks >> 8);
    Wire.endTransmission();
}

uint16_t channelToTicks(int16_t value) {
    if (value > 1000) value = 1000;
    if (value < -1000) value = -1000;
    return SERVO_MID + ((int32_t)value * (SERVO_MAX - SERVO_MIN)) / 2000;
}

uint16_t throttleToTicks(int16_t value) {
    if (value < 0) value = 0;
    if (value > 1000) value = 1000;
    return SERVO_MIN + ((int32_t)value * (SERVO_MAX - SERVO_MIN)) / 1000;
}

void drawPanel(uint8_t px, uint8_t py, const char* label, int16_t value, bool isThrottle) {
    // Panel is 64x32 each
    // Draw border
    u8g2.drawFrame(px, py, 64, 32);
    
    // Label at top
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(px + 3, py + 10, label);
    
    // Value
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", value);
    u8g2.setFont(u8g2_font_7x14B_tn);
    u8g2.drawStr(px + 3, py + 26, buf);
    
    // Mini bar graph on right side of panel
    uint8_t barX = px + 45;
    uint8_t barW = 15;
    uint8_t barH = 24;
    uint8_t barY = py + 4;
    
    u8g2.drawFrame(barX, barY, barW, barH);
    
    uint8_t fillH;
    if (isThrottle) {
        // 0-1000 maps to full bar
        fillH = (value * barH) / 1000;
    } else {
        // -1000 to +1000 maps to half = center
        fillH = ((value + 1000) * barH) / 2000;
    }
    if (fillH > barH) fillH = barH;
    
    // Fill from bottom
    u8g2.drawBox(barX + 1, barY + barH - fillH, barW - 2, fillH);
}

void updateDisplay() {
    u8g2.clearBuffer();
    
    // 4 panels in 2x2 grid
    // Top-left: Roll (0,0)
    // Top-right: Pitch (64,0)
    // Bottom-left: Throttle (0,32)
    // Bottom-right: Yaw (64,32)
    
    drawPanel(0,  0,  "ROLL", pkt.channels[0], false);
    drawPanel(64, 0,  "PTCH", pkt.channels[1], false);
    drawPanel(0,  32, "THRT", pkt.channels[2], true);
    drawPanel(64, 32, "YAW",  pkt.channels[3], false);
    
    u8g2.sendBuffer();
}

void setup() {
    Serial.begin(115200);
    
    // Init OLED
    pinMode(VEXT, OUTPUT);
    digitalWrite(VEXT, LOW);
    delay(100);
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(20, 35, "RXB Starting...");
    u8g2.sendBuffer();
    
    // Init PCA9685
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);
    pcaInit();
    
    // Center servos, throttle to min
    setServo(0, SERVO_MID);
    setServo(1, SERVO_MID);
    setServo(2, SERVO_MIN);
    setServo(3, SERVO_MID);
    
    // Init radio
    SPI.begin(9, 11, 10, 8);
    radio.begin(915.0, 500.0, 6, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 1.6, false);
    radio.setDio1Action(onRx);
    radio.startReceive();
    
    // Zero out packet
    memset(&pkt, 0, sizeof(pkt));
    
    Serial.println("RXB ready");
    delay(500);
}

void loop() {
    if (rxFlag) {
        rxFlag = false;
        if (radio.readData((uint8_t*)&pkt, sizeof(pkt)) == RADIOLIB_ERR_NONE) {
            setServo(0, channelToTicks(pkt.channels[0]));
            setServo(1, channelToTicks(pkt.channels[1]));
            setServo(2, throttleToTicks(pkt.channels[2]));
            setServo(3, channelToTicks(pkt.channels[3]));
        }
        radio.startReceive();
    }
    
    // Update display at ~30fps max to not bog down the loop
    if (millis() - lastDisplayUpdate > 33) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
}