#pragma once

#include <Arduino.h>

class NGGauge
{
public:
    NGGauge(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:
    bool    _initialised;
    uint8_t _pin1, _pin2, _pin3;
    
// Function declarations
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  setInstrumentBrightnessRatio(float ratio);
    void  setNG(float value);
    void  setPowerSave(bool enabled);
    void  drawGauge();

    // Variables
    float    NG = 0;                   // Gas Generator Speed (NG) value from sim
    float    needleRotationAngle       = 0; // angle of rotation of needle based on Fuel Flow from sim  
    float minGreenNG = 55;
    float maxGreenNG = 105;
    float redlineNG = 105;
    float minGreenAngle = 0;
    float maxGreenAngle = 0;
    float redLineAngle = 0;

    float    instrumentBrightness      = 255;
    float    instrumentBrightnessRatio = 0;
    bool     powerSaveFlag             = false;
    int      prevScreenRotation        = 3;
    bool     showLogo                  = true;
    uint16_t backlight_pin = 9;

    int oneValue = 0;
    int tenValue = 0;
    int hundredValue = 0;
    int thousandValue = 0;
};