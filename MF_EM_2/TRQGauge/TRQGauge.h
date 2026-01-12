#pragma once

#include <Arduino.h>

class TRQGauge
{
public:
    TRQGauge(uint8_t Pin1, uint8_t Pin2);
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
    void  setInstrumentBrightness(float value);
    void  setTRQ(float value);
    void  setPowerSave(bool enabled);
    void  drawGauge();
    void  setTRQGreenArcStart(float value);
    void  setTRQGreenArcEnd(float value);
    void  setTRQYellowArcStart(float value);
    void  setTRQYellowArcEnd(float value);
    void  setTRQRedline(float value);


    // Variables
    float    TRQ = 0;                   // Torque value from sim
    float    needleRotationAngle       = 0; // angle of rotation of needle based on Fuel Flow from sim  
    float minGreenTRQ = 400;
    float maxGreenTRQ = 2400;
    float minYellowTRQ = 2400;
    float maxYellowTRQ = 2500;
    float redlineTRQ = 2500;
    float minGreenAngle = 0;
    float maxGreenAngle = 0;
    float minYellowAngle = 0;
    float maxYellowAngle = 0;
    float redLineAngle = 0;

    float    instrumentBrightness      = 255;
    bool     powerSaveFlag             = false;
    int      prevScreenRotation        = 3;
    bool     showLogo                  = true;
    uint16_t backlight_pin = 9;

    int oneValue = 0;
    int tenValue = 0;
    int hundredValue = 0;
    int thousandValue = 0;
};