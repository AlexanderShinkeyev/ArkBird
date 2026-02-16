// TXB with progressive throttle + LED + safe deadzone + OLED @ 80fps + page system + calibration
#include <Arduino.h>
#include <RadioLib.h>
#include "OLED.h"

SX1262 radio = new Module(8, 14, 12, 13);

#define JOY1_X 2
#define JOY1_Y 3
#define JOY2_X 4
#define JOY2_Y 5
#define LED_PIN 48
#define BTN_45 45
#define BTN_46 46

#define THROTTLE_DEADZONE 150
#define NUM_PAGES 3

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

// Page system
uint8_t currentPage = 0;
bool lastBtn45 = false;
bool lastBtn46 = false;

// Calibration offsets (applied to roll, pitch, yaw)
int16_t offsetRoll = 0;
int16_t offsetPitch = 0;
int16_t offsetYaw = 0;

int16_t readAxisRaw(int pin) {
    int raw = analogRead(pin);
    return ((raw - 2048) * 1000) / 550;
}

int16_t readAxis(int pin, int16_t offset) {
    int16_t raw = readAxisRaw(pin);
    int16_t calibrated = raw - offset;
    // Clamp to -1000 to 1000
    if (calibrated > 1000) calibrated = 1000;
    if (calibrated < -1000) calibrated = -1000;
    return calibrated;
}

void setup() {
    Serial.begin(115200);
    
    oledInit();
    oledShowStartup("TXB Starting...");
    
    // Button inputs with pull-up
    pinMode(BTN_45, INPUT_PULLUP);
    pinMode(BTN_46, INPUT_PULLUP);
    
    ledcSetup(0, 5000, 8);
    ledcAttachPin(LED_PIN, 0);
    
    SPI.begin(9, 11, 10, 8);
    radio.begin(915.0, 500.0, 6, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 1.6, false);
    
    Serial.println("TXB 30fps mode");
    delay(500);
}

void loop() {
    // Read raw values for calibration display
    int16_t rawRoll = readAxisRaw(JOY1_X);
    int16_t rawPitch = readAxisRaw(JOY1_Y);
    int16_t rawYaw = readAxisRaw(JOY2_Y);
    
    // Build packet with calibrated values
    ControlPacket pkt;
    pkt.channels[0] = readAxis(JOY1_X, offsetRoll);
    pkt.channels[1] = readAxis(JOY1_Y, offsetPitch);
    pkt.channels[3] = readAxis(JOY2_Y, offsetYaw);
    
    int16_t throttleInput = readAxisRaw(JOY2_X);  // Throttle doesn't use center offset
    
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
    
    // Read button states (inverted because INPUT_PULLUP - pressed = LOW)
    bool btn45 = !digitalRead(BTN_45);
    bool btn46 = !digitalRead(BTN_46);
    
    // BTN 45: Cycle pages on rising edge
    if (btn45 && !lastBtn45) {
        currentPage = (currentPage + 1) % NUM_PAGES;
    }
    lastBtn45 = btn45;
    
    // BTN 46: Set center offsets when on calibration page (page 2)
    if (btn46 && !lastBtn46 && currentPage == 2) {
        offsetRoll = rawRoll;
        offsetPitch = rawPitch;
        offsetYaw = rawYaw;
        Serial.printf("Calibrated: R=%d P=%d Y=%d\n", offsetRoll, offsetPitch, offsetYaw);
    }
    lastBtn46 = btn46;
    
    // Update FPS counter every second
    if (millis() - lastFpsUpdate >= 1000) {
        currentFps = frameCount;
        frameCount = 0;
        lastFpsUpdate = millis();
    }
    
    // 30fps = 33ms per frame (more reliable)
    if (millis() - lastDisplayUpdate > 33) {
        if (currentPage == 2) {
            // Calibration page
            oledUpdateCalibration(rawRoll, rawPitch, rawYaw, 
                                  offsetRoll, offsetPitch, offsetYaw);
        } else {
            // Normal TX display (page 0 and 1 show same for now)
            oledUpdateTX(pkt.channels[0], pkt.channels[1], throttleOut, pkt.channels[3], 
                         throttleValue, currentFps, currentPage);
        }
        lastDisplayUpdate = millis();
        frameCount++;
    }
}