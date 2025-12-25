#include <TFT_eSPI.h>
#include "NPGauge.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "include/NP_Gauge.h"
#include "../Common/DotMatrix_Regular-30.h"
#include "../Common/Needle.h"
#include "../Common/Red_led.h"
#include "../Common/Red_marker.h"

#define BACKGROUND_COLOR  0x1041

static TFT_eSPI    tft;
static TFT_eSprite mainGaugeSpr = TFT_eSprite(&tft);
static TFT_eSprite needleSpr = TFT_eSprite(&tft);
static TFT_eSprite redLEDSpr = TFT_eSprite(&tft);
static TFT_eSprite redMarkerSpr = TFT_eSprite(&tft);

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

NPGauge::NPGauge(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void NPGauge::begin()
{
}

void NPGauge::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;

    // tft = &_tft;
    tft.init();
    tft.setRotation(0);
    tft.setPivot(120, 120);
    tft.fillScreen(TFT_BLACK);
    tft.startWrite(); // TFT chip select held low permanently

    

    mainGaugeSpr.createSprite(NP_GAUGE_WIDTH, NP_GAUGE_HEIGHT);
    mainGaugeSpr.setPivot(120, 120);
    mainGaugeSpr.loadFont(DotMatrix_Regular_30);
    mainGaugeSpr.setTextColor(TFT_GREEN);
    mainGaugeSpr.setTextDatum(TR_DATUM);

    needleSpr.createSprite(NEEDLE_WIDTH, NEEDLE_HEIGHT);
    needleSpr.setPivot(NEEDLE_WIDTH / 2, 80);
    needleSpr.pushImage(0, 0, NEEDLE_WIDTH, NEEDLE_HEIGHT, Needle);

    redLEDSpr.createSprite(RED_LED_WIDTH, RED_LED_HEIGHT);
    redLEDSpr.pushImage(0, 0, RED_LED_WIDTH, RED_LED_HEIGHT, Red_led);

    redMarkerSpr.createSprite(RED_MARKER_WIDTH, RED_MARKER_HEIGHT);
    redMarkerSpr.setPivot(RED_MARKER_WIDTH / 2, 110);
    redMarkerSpr.pushImage(0, 0, RED_MARKER_WIDTH, RED_LED_HEIGHT, Red_marker);

}

void NPGauge::detach()
{
    if (!_initialised)
        return;

    mainGaugeSpr.deleteSprite();
    needleSpr.deleteSprite();
    redMarkerSpr.deleteSprite();
    redLEDSpr.deleteSprite();
    tft.endWrite();

    _initialised = false;
}

void NPGauge::set(int16_t messageID, char *setPoint)
{
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -2 will be send from the board when PowerSavingMode is set
            Message will be "0" for leaving and "1" for entering PowerSavingMode
        MessageID == -1 will be send from the connector when Connector stops running
        Put in your code to enter this mode (e.g. clear a display)

    ********************************************************************************** */

    // do something according your messageID
    switch (messageID) {
    case -1:
        setPowerSave(true);
    case -2:
        setPowerSave((bool)atoi(setPoint));
        break;
    case 0:
        setRPM(atof(setPoint));
        break;
    case 1:
        setInstrumentBrightnessRatio(atof(setPoint));
        break;
    // case 100:
    //     setScreenRotation(atoi(setPoint));
    // break;
    default:
        break;
    }

    // draw the Fuel Flow Gauge
    drawGauge();
}

void NPGauge::update()
{
    // Do something which is required regulary
}

void NPGauge::drawGauge()
{

    oneValue = (int)RPM % 10;
    tenValue = (int)(RPM / 10) % 10;
    hundredValue = (int)(RPM / 100) % 10;
    thousandValue = (int)(RPM / 1000) % 10;

    minGreenAngle = scaleValue(minGreenRPM, 0, 2400, -110, 110);
    maxGreenAngle = scaleValue(maxGreenRPM, 0, 2400, -110, 110);
    redlineRPMAngle = scaleValue(redlineRPM, 0, 2400, -110, 110);
    needleRotationAngle = scaleValue(RPM, 0, 2400, -110, 110);
    
    mainGaugeSpr.fillSprite(TFT_BLACK);
    mainGaugeSpr.pushImage(0, 0, NP_GAUGE_WIDTH, NP_GAUGE_HEIGHT, NP_Gauge);
    mainGaugeSpr.drawSmoothArc(120, 120, 205 / 2, 195 / 2, minGreenAngle + 180, maxGreenAngle + 180, TFT_GREEN, TFT_BLACK);

    // Draw the numbers in the digital display
    mainGaugeSpr.drawString(String(oneValue), 162, 170);
    if (RPM >= 10)
        mainGaugeSpr.drawString(String(tenValue), 140, 170);
    if (RPM >= 100)
        mainGaugeSpr.drawString(String(hundredValue), 119, 170);
    if (RPM >= 1000)
        mainGaugeSpr.drawString(String(thousandValue), 97, 170);

    if (RPM >= redlineRPM )
        redLEDSpr.pushToSprite(&mainGaugeSpr, 38, 159, BACKGROUND_COLOR);
    
    // Draw the red line marker
    redMarkerSpr.pushRotated(&mainGaugeSpr, redlineRPMAngle, BACKGROUND_COLOR);

    // Draw the needle
    needleSpr.pushRotated(&mainGaugeSpr, needleRotationAngle, BACKGROUND_COLOR);

    // Final draw everything
    mainGaugeSpr.pushSprite(0, 0);

}

void NPGauge::setRPM(float value)
{
    RPM = value;
}

void NPGauge::setInstrumentBrightnessRatio(float ratio)
{
    instrumentBrightnessRatio = ratio;
    instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
    analogWrite(backlight_pin, instrumentBrightness);
}

void NPGauge::setPowerSave(bool enabled)
{
    if (enabled) {
        analogWrite(backlight_pin, 0);
        powerSaveFlag = true;
    } else {
        analogWrite(backlight_pin, instrumentBrightness);
        powerSaveFlag = false;
    }
}


float NPGauge::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}