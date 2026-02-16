#include "hardware/det.h"
#include <Arduino.h>
#include <RadioLib.h>

// Pins
#define JOY1_X 2
#define JOY1_Y 3
#define JOY2_X 5
#define JOY2_Y 4
#define LED_PIN 48
#define BTN_39 39
#define BTN_41 41
#define BTN_33 33
#define BTN_47 47

#define THROTTLE_DEADZONE 150

static SX1262 radio = new Module(8, 14, 12, 13);

struct ControlPacket {
    int16_t channels[4];
    uint8_t sequence;
};

static uint8_t seq = 0;

static int16_t readAxisRaw(int pin) {
    int raw = analogRead(pin);
    return ((raw - 2048) * 1000) / 2048;
}

static int16_t readAxis(int pin, int16_t offset) {
    int16_t raw = readAxisRaw(pin);
    int16_t calibrated = raw - offset;
    if (calibrated > 1000) calibrated = 1000;
    if (calibrated < -1000) calibrated = -1000;
    return calibrated;
}

void detInit() {
    pinMode(BTN_39, INPUT_PULLUP);
    pinMode(BTN_41, INPUT_PULLUP);
    pinMode(BTN_33, INPUT_PULLUP);
    pinMode(BTN_47, INPUT_PULLUP);
    
    ledcSetup(0, 5000, 8);
    ledcAttachPin(LED_PIN, 0);
    
    SPI.begin(9, 11, 10, 8);
    radio.begin(915.0, 500.0, 6, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 1.6, false);
}

void detUpdate(TXState* s) {
    // Raw axes
    s->rawRoll = readAxisRaw(JOY1_X);
    s->rawPitch = readAxisRaw(JOY1_Y);
    s->rawYaw = readAxisRaw(JOY2_Y);
    
    // Calibrated axes
    s->roll = readAxis(JOY1_X, s->offsetRoll);
    s->pitch = readAxis(JOY1_Y, s->offsetPitch);
    s->yaw = readAxis(JOY2_Y, s->offsetYaw);
    
    // Throttle
    int16_t throttleInput = readAxisRaw(JOY2_X);
    if (throttleInput > 100 || throttleInput < -100) {
        s->throttleValue += throttleInput * 0.002f;
    }
    if (s->throttleValue < 0) s->throttleValue = 0;
    if (s->throttleValue > 1000) s->throttleValue = 1000;
    s->throttle = (s->throttleValue < THROTTLE_DEADZONE) ? 0 : s->throttleValue;
    
    // LED
    ledcWrite(0, s->throttleValue * 255 / 1000);
    
    // Transmit
    ControlPacket pkt;
    pkt.channels[0] = s->roll;
    pkt.channels[1] = s->pitch;
    pkt.channels[2] = s->throttle;
    pkt.channels[3] = s->yaw;
    pkt.sequence = seq++;
    radio.transmit((uint8_t*)&pkt, sizeof(pkt));
    
    // Buttons
    s->btn39 = analogRead(BTN_39) > 500;
    s->btn41 = analogRead(BTN_41) > 500;
    s->btn33 = !digitalRead(BTN_33);
    s->btn47 = !digitalRead(BTN_47);
    
    // Extra axes
    s->axis39 = ((analogRead(BTN_39) - 2048) * 1000) / 2048;
    s->axis41 = ((analogRead(BTN_41) - 2048) * 1000) / 2048;
}