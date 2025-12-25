#include <TFT_eSPI.h>
#include "FFGauge.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "include/FF_Gauge.h"
#include "../Common/DotMatrix_Regular-30.h"
#include "../Common/Needle.h"
#include "../Common/Red_led.h"

#define BACKGROUND_COLOR  0x1041

static TFT_eSPI    tft;
static TFT_eSprite mainGaugeSpr = TFT_eSprite(&tft);
static TFT_eSprite needleSpr = TFT_eSprite(&tft);;

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

FFGauge::FFGauge(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void FFGauge::begin()
{
}

void FFGauge::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;

    // tft = &_tft;
    tft.init();
    tft.setRotation(0);
    tft.setPivot(120, 120);
    tft.fillScreen(TFT_BLACK);
    tft.startWrite(); // TFT chip select held low permanently

    

    mainGaugeSpr.createSprite(FF_GAUGE_WIDTH, FF_GAUGE_HEIGHT);
    mainGaugeSpr.setPivot(120, 120);
    mainGaugeSpr.loadFont(DotMatrix_Regular_30);
    mainGaugeSpr.setTextColor(TFT_GREEN);
    mainGaugeSpr.setTextDatum(TR_DATUM);

    needleSpr.createSprite(NEEDLE_WIDTH, NEEDLE_HEIGHT);
    needleSpr.setPivot(NEEDLE_WIDTH / 2, 80);
    needleSpr.pushImage(0, 0, NEEDLE_WIDTH, NEEDLE_HEIGHT, Needle);

}

void FFGauge::detach()
{
    if (!_initialised)
        return;

    mainGaugeSpr.deleteSprite();
    needleSpr.deleteSprite();
    tft.endWrite();

    _initialised = false;
}

void FFGauge::set(int16_t messageID, char *setPoint)
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
        setFuelFlow(atof(setPoint));
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

void FFGauge::update()
{
    // Do something which is required regulary
}

void FFGauge::drawGauge()
{

    oneValue = (int)fuelFlow % 10;
    tenValue = (int)(fuelFlow / 10) % 10;
    hundredValue = (int)(fuelFlow / 100) % 10;
    thousandValue = (int)(fuelFlow / 1000) % 10;

    needleRotationAngle = scaleValue(fuelFlow, 0, 700, -110, 110);
    mainGaugeSpr.fillSprite(TFT_BLACK);
    mainGaugeSpr.pushImage(0, 0, 240, 240, FF_Gauge);

    // Draw the numbers in the digital display
    mainGaugeSpr.drawString(String(oneValue), 162, 170);
    if (fuelFlow >= 10)
        mainGaugeSpr.drawString(String(tenValue), 140, 170);
    if (fuelFlow >= 100)
        mainGaugeSpr.drawString(String(hundredValue), 119, 170);
    if (fuelFlow >= 1000)
        mainGaugeSpr.drawString(String(thousandValue), 97, 170);

    // Draw the needle
    needleSpr.pushRotated(&mainGaugeSpr, needleRotationAngle, BACKGROUND_COLOR);



}

void FFGauge::setFuelFlow(float value)
{
    fuelFlow = value;
}

void FFGauge::setInstrumentBrightnessRatio(float ratio)
{
    instrumentBrightnessRatio = ratio;
    instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
    analogWrite(backlight_pin, instrumentBrightness);
}

void FFGauge::setPowerSave(bool enabled)
{
    if (enabled) {
        analogWrite(backlight_pin, 0);
        powerSaveFlag = true;
    } else {
        analogWrite(backlight_pin, instrumentBrightness);
        powerSaveFlag = false;
    }
}


float FFGauge::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}