// TXB with progressive throttle + LED + safe deadzone + OLED @ 80fps
#include <Arduino.h>
#include <RadioLib.h>
#include "OLED.h"

SX1262 radio = new Module(8, 14, 12, 13);

#define JOY1_X 1
#define JOY1_Y 3
#define JOY2_X 7
#define JOY2_Y 5
#define LED_PIN 48

#define THROTTLE_DEADZONE 150

struct ControlPacket {
    int16_t channels[4];
    uint8_t sequence;
};

uint8_t seq = 0;
float throttleValue = 0;
unsigned long lastDisplayUpdate = 0;

// FPS tracking
uint16_t frameCount = 0;
uint16_t currentFps = 0;
unsigned long lastFpsUpdate = 0;

int16_t readAxis(int pin) {
    int raw = analogRead(pin);
    return ((raw - 2048) * 1000) / 2048;
}

void setup() {
    Serial.begin(115200);
    
    oledInit();
    oledShowStartup("TXB Starting...");
    
    ledcSetup(0, 5000, 8);
    ledcAttachPin(LED_PIN, 0);
    
    SPI.begin(9, 11, 10, 8);
    radio.begin(915.0, 500.0, 6, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 1.6, false);
    
    Serial.println("TXB 80fps mode");
    delay(500);
}

void loop() {
    ControlPacket pkt;
    pkt.channels[0] = readAxis(JOY1_X);
    pkt.channels[1] = readAxis(JOY1_Y);
    pkt.channels[3] = readAxis(JOY2_Y);
    
    int16_t throttleInput = readAxis(JOY2_X);
    
    if (throttleInput > 100 || throttleInput < -100) {
        throttleValue += throttleInput * 0.002;
    }
    
    if (throttleValue < 0) throttleValue = 0;
    if (throttleValue > 1000) throttleValue = 1000;
    
    int16_t throttleOut = (throttleValue < THROTTLE_DEADZONE) ? 0 : throttleValue;
    
    pkt.channels[2] = throttleOut;
    pkt.sequence = seq++;
    
    ledcWrite(0, throttleValue * 255 / 1000);
    
    radio.transmit((uint8_t*)&pkt, sizeof(pkt));
    
    // Update FPS counter every second
    if (millis() - lastFpsUpdate >= 1000) {
        currentFps = frameCount;
        frameCount = 0;
        lastFpsUpdate = millis();
    }
    
    // 80fps = 12.5ms per frame
    if (millis() - lastDisplayUpdate > 12) {
        oledUpdateTX(pkt.channels[0], pkt.channels[1], throttleOut, pkt.channels[3], throttleValue, currentFps);
        lastDisplayUpdate = millis();
        frameCount++;
    }
}