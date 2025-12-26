#include <TFT_eSPI.h>
#include "OTOPGauge.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "include/OTOP_Gauge.h"
#include "include/OTOP_Needle.h"
#include "../Common/Needle.h"
#include "../Common/Red_led.h"
#include "../Common/Red_marker.h"

#define BACKGROUND_COLOR  0x1041

static TFT_eSPI    tft;
static TFT_eSprite mainGaugeSpr = TFT_eSprite(&tft);
static TFT_eSprite needleOTOPSpr = TFT_eSprite(&tft);
static TFT_eSprite redLEDSpr = TFT_eSprite(&tft);
static TFT_eSprite redMarkerSpr = TFT_eSprite(&tft);

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

OTOPGauge::OTOPGauge(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void OTOPGauge::begin()
{
}

void OTOPGauge::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;

    // tft = &_tft;
    tft.init();
    tft.setRotation(0);
    tft.setPivot(120, 120);
    tft.fillScreen(TFT_BLACK);
    tft.startWrite(); // TFT chip select held low permanently

    mainGaugeSpr.createSprite(OTOP_GAUGE_WIDTH, OTOP_GAUGE_HEIGHT);
    mainGaugeSpr.setPivot(120, 120);

    needleOTOPSpr.createSprite(OTOP_NEEDLE_WIDTH, OTOP_NEEDLE_HEIGHT);
    needleOTOPSpr.setPivot(OTOP_NEEDLE_WIDTH / 2, 85);
    needleOTOPSpr.fillSprite(BACKGROUND_COLOR);
    needleOTOPSpr.pushImage(0, 0, OTOP_NEEDLE_WIDTH, OTOP_GAUGE_HEIGHT, OTOP_Needle);

    redLEDSpr.createSprite(RED_LED_WIDTH, RED_LED_HEIGHT);
    redLEDSpr.pushImage(0, 0, RED_LED_WIDTH, RED_LED_HEIGHT, Red_led);

    redMarkerSpr.createSprite(RED_MARKER_WIDTH, RED_MARKER_HEIGHT);
    redMarkerSpr.setPivot(RED_MARKER_WIDTH / 2, 110);
    redMarkerSpr.pushImage(0, 0, RED_MARKER_WIDTH, RED_LED_HEIGHT, Red_marker);

}

void OTOPGauge::detach()
{
    if (!_initialised)
        return;
    mainGaugeSpr.deleteSprite();
    needleOTOPSpr.deleteSprite();
    redLEDSpr.deleteSprite();
    redMarkerSpr.deleteSprite();
    tft.fillScreen(TFT_BLACK);
    tft.endWrite();

    _initialised = false;
}

void OTOPGauge::set(int16_t messageID, char *setPoint)
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
        setOT(atof(setPoint));
        break;
    case 1:
        setOP(atof(setPoint));
        break;
    case 2:
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

void OTOPGauge::update()
{
    // Do something which is required regulary
}

void OTOPGauge::drawGauge()
{
    mainGaugeSpr.pushImage(0, 0, OTOP_GAUGE_WIDTH, OTOP_GAUGE_HEIGHT, OTOP_Gauge);
    drawOTGauge();
    drawOPGauge();
    mainGaugeSpr.pushSprite(0, 0);
}

void OTOPGauge::drawOTGauge()
{

    minRedLineAngleOT = scaleValue(minRedLineOT, -50, 150, -145, -35);
    maxRedLineAngleOT = scaleValue(maxRedLineOT, -50, 150, -145, -35);

    minYellowAngleOT = scaleValue(minYellowOT, -50, 150, -145, -35);
    maxYellowAngleOT = scaleValue(maxYellowOT, -50, 150, -145, -35);

    minGreenAngleOT = scaleValue(minGreenOT, -50, 150, -145, -35);
    maxGreenAngleOT = scaleValue(maxGreenOT, -50, 150, -145, -35);
    
    needleRotationAngleOT = scaleValue(oilTemperature, -50, 150, -145, -35);

    mainGaugeSpr.drawSmoothArc(120, 120, 205 / 2, 195 / 2, minYellowAngleOT + 180, maxYellowAngleOT + 180, TFT_YELLOW, TFT_BLACK); // Draw Yellow Line

    mainGaugeSpr.drawSmoothArc(120, 120, 205 / 2, 195 / 2, minGreenAngleOT + 180, maxGreenAngleOT + 180, TFT_GREEN, TFT_BLACK); // Draw Yellow Line

    needleOTOPSpr.pushRotated(&mainGaugeSpr, needleRotationAngleOT, BACKGROUND_COLOR);

    redMarkerSpr.pushRotated(&mainGaugeSpr, minRedLineAngleOT, BACKGROUND_COLOR); // Draw the minimum red line for Oil Temperature
    redMarkerSpr.pushRotated(&mainGaugeSpr, maxRedLineAngleOT, BACKGROUND_COLOR); // Draw the maximum red line for Oil Temperature

    // Draw the Red LED
    if (oilTemperature <= minRedLineOT || oilTemperature >= maxRedLineOT)
        redLEDSpr.pushToSprite(&mainGaugeSpr, 110, 204, BACKGROUND_COLOR);

}


void OTOPGauge::drawOPGauge()
{
    if (minRedLineOP >= 0 && minRedLineOP < 40)
        minRedLineAngleOP = scaleValue(minRedLineOP, 0, 40, 140, 120);
    else if (minRedLineOP > 120 && minRedLineOP <= 200)
        minRedLineAngleOP = scaleValue(minRedLineOP, 120, 200, 60, 40);
    else
        minRedLineAngleOP = scaleValue(minRedLineOP, 40, 120, 120, 60);

    if (maxRedLineOP >= 0 && maxRedLineOP < 40)
        maxRedLineAngleOP= scaleValue(maxRedLineOP, 0, 40, 140, 120);
    else if (maxRedLineOP > 120 && maxRedLineOP <= 200)
        maxRedLineAngleOP = scaleValue(maxRedLineOP, 120, 200, 60, 40);
    else
        maxRedLineAngleOP = scaleValue(maxRedLineOP, 40, 120, 120, 60);

    if (minYellowOP >= 0 && minYellowOP < 40)
        minYellowAngleOP = scaleValue(minYellowOP, 0, 40, 140, 120);
    else if (minYellowOP > 120 && minYellowOP <= 200)
        minYellowAngleOP = scaleValue(minYellowOP, 120, 200, 60, 40);
    else
        minYellowAngleOP = scaleValue(minYellowOP, 40, 120, 120, 60);

    if (maxYellowOP >= 0 && maxYellowOP < 40)
        maxYellowAngleOP = scaleValue(maxYellowOP, 0, 40, 140, 120);
    else if (maxYellowOP > 120 && maxYellowOP <= 200)
        maxYellowAngleOP = scaleValue(maxYellowOP, 120, 200, 60, 40);
    else
        maxYellowAngleOP = scaleValue(maxYellowOP, 40, 120, 120, 60);

    if (minGreenOP >= 0 && minGreenOP < 40)
        minGreenAngleOP = scaleValue(minGreenOP, 0, 40, 140, 120);
    else if (minGreenOP > 120 && minGreenOP <= 200)
        minGreenAngleOP = scaleValue(minGreenOP, 120, 200, 60, 40);
    else
        minGreenAngleOP = scaleValue(minGreenOP, 40, 120, 120, 60);

    if (maxGreenOP >= 0 && maxGreenOP < 40)
        maxGreenAngleOP = scaleValue(maxGreenOP, 0, 40, 140, 120);
    else if (maxGreenOP > 120 && maxGreenOP <= 200)
        maxGreenAngleOP = scaleValue(maxGreenOP, 120, 200, 60, 40);
    else
        maxGreenAngleOP = scaleValue(maxGreenOP, 40, 120, 120, 60);

    if (oilPressure >= 0 && oilPressure < 40)
        needleRotationAngleOP= scaleValue(oilPressure, 0, 40, 140, 120);
    else if (oilPressure > 120 && oilPressure <= 200)
        needleRotationAngleOP = scaleValue(oilPressure, 120, 200, 60, 40);
    else
        needleRotationAngleOP = scaleValue(oilPressure, 40, 120, 120, 60);

    mainGaugeSpr.drawSmoothArc(120, 120, 205 / 2, 195 / 2, maxYellowAngleOP + 180, minYellowAngleOP + 180, TFT_YELLOW, TFT_BLACK); // Draw Yellow Line

    mainGaugeSpr.drawSmoothArc(120, 120, 205 / 2, 195 / 2, maxGreenAngleOP + 180, minGreenAngleOP + 180, TFT_GREEN, TFT_BLACK); // Draw Yellow Line

    redMarkerSpr.pushRotated(&mainGaugeSpr, minRedLineAngleOP, BACKGROUND_COLOR); // Draw the minimum red line for Oil Temperature
    redMarkerSpr.pushRotated(&mainGaugeSpr, maxRedLineAngleOP, BACKGROUND_COLOR); // Draw the maximum red line for Oil Temperature

    needleOTOPSpr.pushRotated(&mainGaugeSpr, needleRotationAngleOP, BACKGROUND_COLOR);

    // Draw the red led
    if (oilPressure <= minRedLineOP || oilPressure >= maxRedLineOP)
        redLEDSpr.pushToSprite(&mainGaugeSpr, 110, 204, BACKGROUND_COLOR);

}

void OTOPGauge::setOT(float value)
{
    oilTemperature = value;
}

void OTOPGauge::setOP(float value)
{
    oilPressure = value;
}


void OTOPGauge::setInstrumentBrightnessRatio(float ratio)
{
    instrumentBrightnessRatio = ratio;
    instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
    analogWrite(backlight_pin, instrumentBrightness);
}

void OTOPGauge::setPowerSave(bool enabled)
{
    if (enabled) {
        analogWrite(backlight_pin, 0);
        powerSaveFlag = true;
    } else {
        analogWrite(backlight_pin, instrumentBrightness);
        powerSaveFlag = false;
    }
}


float OTOPGauge::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}