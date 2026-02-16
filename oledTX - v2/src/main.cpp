// TXB - Transmitter with OLED display
#include <Arduino.h>
#include "state.h"
#include "hardware/det.h"
#include "hardware/OLED_screen/OLED.h"
#include "hardware/OLED_screen/perm/topbar.h"  // <-- add this
#include "pages/pages.h"

TXState tx = {};

// Display timing
unsigned long lastDisplayUpdate = 0;
uint16_t frameCount = 0;
unsigned long lastFpsUpdate = 0;

// Button edge detection
bool lastBtn33 = false;
bool lastBtn47 = false;

void setup() {
    Serial.begin(115200);
    
    detInit();
    oledInit();
    topbarInitBattery();  // <-- add this
    oledShowStartup("TXB Starting...");
    
    Serial.println("TXB ready");
    delay(500);
}

void loop() {
    detUpdate(&tx);
    tx.axis39 = analogRead(1);
    tx.voltage = topbarReadVoltage();  // <-- add this
    tx.voltageRaw = topbarReadRaw();

    // BTN1 (33) rising edge -> next page
    if (tx.btn33 && !lastBtn33) {
        pageNext();
    }
    lastBtn33 = tx.btn33;
    
    // BTN2 (47) rising edge -> page action
    if (tx.btn47 && !lastBtn47) {
        pageBtn2(&tx);
    }
    lastBtn47 = tx.btn47;
    
    // FPS counter
    if (millis() - lastFpsUpdate >= 1000) {
        tx.fps = frameCount;
        frameCount = 0;
        lastFpsUpdate = millis();
    }
    
    // Display @ 30fps
    if (millis() - lastDisplayUpdate > 33) {
        pageRender(&tx);
        lastDisplayUpdate = millis();
        frameCount++;
    }
}