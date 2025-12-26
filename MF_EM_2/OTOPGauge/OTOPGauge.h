#pragma once

#include <Arduino.h>

class OTOPGauge
{
public:
    OTOPGauge(uint8_t Pin1, uint8_t Pin2);
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
    void  setOT(float value);
    void  setOP(float value);
    void  setPowerSave(bool enabled);
    void  drawOTGauge();
    void  drawOPGauge();
    void  drawGauge();

    // Variables
    float    oilTemperature = 0;                   // Oil Temperature value from sim
    float    oilPressure = 0;                   // Oil Pressure value from sim
    float    needleRotationAngleOT       = 0; // angle of rotation of needle based on Fuel Flow from sim  
    float    needleRotationAngleOP       = 0; // angle of rotation of needle based on Fuel Flow from sim  

    // Oil temperature parameters
    float minGreenOT = 31;
    float maxGreenOT = 100;
    float minYellowOT = -40;
    float maxYellowOT = 30;
    float minGreenAngleOT = 0;
    float maxGreenAngleOT = 0;
    float minYellowAngleOT = 0;
    float maxYellowAngleOT = 0;
    float minRedLineOT = -40;
    float maxRedLineOT = 100;
    float minRedLineAngleOT = 0;
    float maxRedLineAngleOT = 0;

    // Oil pessure parameters
    float minGreenOP = 55;
    float maxGreenOP = 105;
    float minYellowOP = 40;
    float maxYellowOP = 85;
    float minGreenAngleOP = 105;
    float maxGreenAngleOP = 200;
    float minYellowAngleOP = 0;
    float maxYellowAngleOP = 0;
    float minRedLineOP = 40;
    float maxRedLineOP = 200;
    float minRedLineAngleOP = 0;
    float maxRedLineAngleOP = 0;

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