#include <TFT_eSPI.h>
#include "NGGauge.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "include/NG_Gauge.h"
#include "include/DotMatrix_Regular-20.h"
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

NGGauge::NGGauge(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void NGGauge::begin()
{
}

void NGGauge::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;

    // tft = &_tft;
    tft.init();
    tft.setRotation(0);
    tft.setPivot(120, 120);
    tft.fillScreen(TFT_BLACK);
    tft.startWrite(); // TFT chip select held low permanently

    mainGaugeSpr.createSprite(NG_GAUGE_WIDTH, NG_GAUGE_HEIGHT);
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

void NGGauge::detach()
{
    if (!_initialised)
        return;
    mainGaugeSpr.deleteSprite();
    needleSpr.deleteSprite();
    redLEDSpr.deleteSprite();
    redMarkerSpr.deleteSprite();
    tft.fillScreen(TFT_BLACK);
    tft.endWrite();

    _initialised = false;
}

void NGGauge::set(int16_t messageID, char *setPoint)
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
        setNG(atof(setPoint));
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

void NGGauge::update()
{
    // Do something which is required regulary
}

void NGGauge::drawGauge()
{

    int tempNumber = 0; // temporary storage
    int decNumber = 0;  // the one decimal number

    tempNumber = (int) (NG * 10.0);
    decNumber = tempNumber % 10;

    oneValue = (int)NG % 10;
    tenValue = (int)(NG / 10) % 10;
    hundredValue = (int)(NG / 100) % 10;
    thousandValue = (int)(NG / 1000) % 10;
    minGreenAngle = scaleValue(minGreenNG, 0, 110, -110, 110);
    maxGreenAngle = scaleValue(maxGreenNG, 0, 110, -110, 110);
    redLineAngle = scaleValue(redlineNG, 0, 110, -110, 110);
    needleRotationAngle = scaleValue(NG, 0, 110, -110, 110);

    mainGaugeSpr.fillSprite(TFT_BLACK);
    
    mainGaugeSpr.pushImage(0, 0, 240, 240, NG_Gauge);
    // mainGaugeSpr.drawString(String((int)NG), 168, 170);

    // Draw Green Arc
    mainGaugeSpr.drawSmoothArc(120, 120, 205 / 2, 195 / 2, minGreenAngle + 180, maxGreenAngle + 180, TFT_GREEN, BACKGROUND_COLOR);
    // Draw Red Marker
    redMarkerSpr.pushRotated(&mainGaugeSpr, redLineAngle, BACKGROUND_COLOR);

    // Draw the numbers in the digital display
    // numberSpr.pushToSprite(&mainGaugeSpr, 160, 170, BACKGROUND_COLOR);
    mainGaugeSpr.loadFont(DotMatrix_Regular_20);
    mainGaugeSpr.setTextDatum(TR_DATUM);
    mainGaugeSpr.drawString(String(decNumber), 162, 170);
    mainGaugeSpr.loadFont(DotMatrix_Regular_30);
    mainGaugeSpr.setTextDatum(TR_DATUM);
    mainGaugeSpr.drawString(String(oneValue), 140, 170);
    if (NG >= 10)
        mainGaugeSpr.drawString(String(tenValue), 119, 170);
    if (NG >= 100)
        mainGaugeSpr.drawString(String(hundredValue), 97, 170);


    // Draw the needle
    needleSpr.pushRotated(&mainGaugeSpr, needleRotationAngle, BACKGROUND_COLOR);

    // Draw Red Led if red line is crossed
    if (NG >= redlineNG )
        redLEDSpr.pushToSprite(&mainGaugeSpr, 38, 159, BACKGROUND_COLOR);

    mainGaugeSpr.pushSprite(0, 0);

}

void NGGauge::setNG(float value)
{
    NG = value;
}

void NGGauge::setInstrumentBrightnessRatio(float ratio)
{
    instrumentBrightnessRatio = ratio;
    instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
    analogWrite(backlight_pin, instrumentBrightness);
}

void NGGauge::setPowerSave(bool enabled)
{
    if (enabled) {
        analogWrite(backlight_pin, 0);
        powerSaveFlag = true;
    } else {
        analogWrite(backlight_pin, instrumentBrightness);
        powerSaveFlag = false;
    }
}


float NGGauge::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}