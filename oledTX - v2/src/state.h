#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

struct TXState {
    // Calibrated outputs
    int16_t roll;
    int16_t pitch;
    int16_t throttle;
    int16_t yaw;
    
    // Raw inputs (for calibration display)
    int16_t rawRoll;
    int16_t rawPitch;
    int16_t rawYaw;
    
    // Calibration offsets
    int16_t offsetRoll;
    int16_t offsetPitch;
    int16_t offsetYaw;
    
    // Throttle state
    float throttleValue;
    
    // Extra axes
    int16_t axis39;
    int16_t axis41;
    
    // Buttons
    bool btn33;
    bool btn47;
    bool btn39;
    bool btn41;
    
    // Display
    uint16_t fps;
    uint8_t page;
    
    // Battery
    float voltage;
    uint16_t voltageRaw;
};

extern TXState tx;

#endif