#include <TFT_eSPI.h>
#include "TRQGauge.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "include/TRQ_Gauge.h"
#include "include/TRQ_Dotted_Line.h"
#include "../Common/DotMatrix_Regular-30.h"
#include "../Common/Needle.h"
#include "../Common/Red_led.h"
#include "../Common/Red_marker.h"
#include "RunningAverage.h"

#define BACKGROUND_COLOR  0x1041

static TFT_eSPI    tft;
static TFT_eSprite mainGaugeSpr = TFT_eSprite(&tft);
static TFT_eSprite needleSpr = TFT_eSprite(&tft);
static TFT_eSprite redLEDSpr = TFT_eSprite(&tft);
static TFT_eSprite redMarkerSpr = TFT_eSprite(&tft);
static TFT_eSprite dottedLineSpr = TFT_eSprite(&tft);

int TRQMessageID = -100;  // Just for tracking the messageID value
RunningAverage RA_RPMNeedleRotationAngle(5);  // Running average of last 5 values of the angle rotation

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

TRQGauge::TRQGauge(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void TRQGauge::begin()
{
}

void TRQGauge::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;

    // tft = &_tft;
    tft.init();
    tft.setRotation(0);
    tft.setPivot(120, 120);
    tft.fillScreen(TFT_BLACK);
    tft.startWrite(); // TFT chip select held low permanently

    mainGaugeSpr.createSprite(TRQ_GAUGE_WIDTH, TRQ_GAUGE_HEIGHT);
    mainGaugeSpr.setPivot(120, 120);
    mainGaugeSpr.loadFont(DotMatrix_Regular_30);
    mainGaugeSpr.setTextColor(TFT_GREEN);
    mainGaugeSpr.setTextDatum(TC_DATUM);

    needleSpr.createSprite(NEEDLE_WIDTH, NEEDLE_HEIGHT);
    needleSpr.setPivot(NEEDLE_WIDTH / 2, 80);
    needleSpr.pushImage(0, 0, NEEDLE_WIDTH, NEEDLE_HEIGHT, Needle);

    redLEDSpr.createSprite(RED_LED_WIDTH, RED_LED_HEIGHT);
    redLEDSpr.pushImage(0, 0, RED_LED_WIDTH, RED_LED_HEIGHT, Red_led);

    redMarkerSpr.createSprite(RED_MARKER_WIDTH, RED_MARKER_HEIGHT);
    redMarkerSpr.setPivot(RED_MARKER_WIDTH / 2, 110);
    redMarkerSpr.pushImage(0, 0, RED_MARKER_WIDTH, RED_LED_HEIGHT, Red_marker);

    dottedLineSpr.createSprite(TRQ_DOTTED_LINE_WIDTH, TRQ_DOTTED_LINE_HEIGHT);
    dottedLineSpr.setPivot(TRQ_DOTTED_LINE_WIDTH / 2, 105);
    dottedLineSpr.pushImage(0, 0, TRQ_DOTTED_LINE_WIDTH, TRQ_DOTTED_LINE_HEIGHT, TRQ_Dotted_Line);

    // Clear Running Average
    RA_RPMNeedleRotationAngle.clear();

}

void TRQGauge::detach()
{
    if (!_initialised)
        return;
    mainGaugeSpr.deleteSprite();
    needleSpr.deleteSprite();
    redLEDSpr.deleteSprite();
    redMarkerSpr.deleteSprite();
    dottedLineSpr.deleteSprite();
    tft.fillScreen(TFT_BLACK);
    tft.endWrite();

    _initialised = false;
}

void TRQGauge::set(int16_t messageID, char *setPoint)
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
    TRQMessageID = messageID;
    // do something according your messageID
    switch (messageID) {
    case -1:
        setPowerSave(true);
    case -2:
        setPowerSave((bool)atoi(setPoint));
        break;
    case 0:
        setTRQ(atof(setPoint));
        break;
    case 1:
        setTRQGreenArcStart(atof(setPoint));
        break;
    case 2:
        setTRQGreenArcEnd(atof(setPoint));
        break;
    case 3:
        setTRQYellowArcStart(atof(setPoint));
        break;
    case 4:
        setTRQYellowArcEnd(atof(setPoint));
        break;
    case 5:
        setTRQRedline(atof(setPoint));
        break;
    case 100:
        setInstrumentBrightness(atof(setPoint));
        break;
    default:
        break;
    }
}

void TRQGauge::update()
{
    // Do something which is required regulary
    if (TRQMessageID == -1 || powerSaveFlag == true)  // Mobiflight Connector has stopped or entered power save mode
    {
        tft.fillScreen(TFT_BLACK);
        analogWrite(TFT_BL, 0);
    }
    else
    {
        float pwmOutput = 0;
        pwmOutput = sq(instrumentBrightness) / 255.0;  // needed to correct PWM output due to human eye brightness perception
        analogWrite(TFT_BL, pwmOutput);
        drawGauge();
    }

}

void TRQGauge::drawGauge()
{

    oneValue = (int)TRQ % 10;
    tenValue = (int)(TRQ / 10) % 10;
    hundredValue = (int)(TRQ / 100) % 10;
    thousandValue = (int)(TRQ / 1000) % 10;
    minGreenAngle = scaleValue(minGreenTRQ, 0, 2700, -110, 110);
    maxGreenAngle = scaleValue(maxGreenTRQ, 0, 2700, -110, 110);
    minYellowAngle = scaleValue(minYellowTRQ, 0, 2700, -110, 110);
    maxYellowAngle = scaleValue(maxYellowTRQ, 0, 2700, -110, 110);
    redLineAngle = scaleValue(redlineTRQ, 0, 2700, -110, 110);
    needleRotationAngle = scaleValue(TRQ, 0, 2700, -110, 110);

    // Add needleRotationAngle value to running average
    RA_RPMNeedleRotationAngle.addValue(needleRotationAngle);

    mainGaugeSpr.fillSprite(TFT_BLACK);
    
    mainGaugeSpr.pushImage(0, 0, 240, 240, TRQ_Gauge);
    // mainGaugeSpr.drawString(String((int)TRQ), 168, 170);

    // Draw Green Arc
    mainGaugeSpr.drawSmoothArc(120, 120, 205 / 2, 195 / 2, minGreenAngle + 180, maxGreenAngle + 180, TFT_GREEN, BACKGROUND_COLOR);

    // Draw Yellow Arc
    mainGaugeSpr.drawSmoothArc(120, 120, 205 / 2, 195 / 2, minYellowAngle + 180, maxYellowAngle + 180, TFT_YELLOW, BACKGROUND_COLOR);

    // Draw Red Marker
    redMarkerSpr.pushRotated(&mainGaugeSpr, minYellowAngle, BACKGROUND_COLOR);

    // Draw Start Limits Marker
    dottedLineSpr.pushRotated(&mainGaugeSpr, redLineAngle, BACKGROUND_COLOR);

    // Draw the numbers in the digital display
    mainGaugeSpr.drawString(String(oneValue), 160, 170);
    if (TRQ >= 10)
        mainGaugeSpr.drawString(String(tenValue), 138, 170);
    if (TRQ >= 100)
        mainGaugeSpr.drawString(String(hundredValue), 116, 170);
    if (TRQ >= 1000)
        mainGaugeSpr.drawString(String(thousandValue), 94, 170);

    // Draw the needle
    needleSpr.pushRotated(&mainGaugeSpr, RA_RPMNeedleRotationAngle.getAverage(), BACKGROUND_COLOR);

    // Draw Red Led if red line is crossed
    if (TRQ >= redlineTRQ )
        redLEDSpr.pushToSprite(&mainGaugeSpr, 38, 159, BACKGROUND_COLOR);

    mainGaugeSpr.pushSprite(0, 0);

}


// Setters
void TRQGauge::setTRQ(float value)
{
    TRQ = value;
}

void  TRQGauge::setTRQGreenArcStart(float value)
{
    minGreenTRQ = value;
}

void  TRQGauge::setTRQGreenArcEnd(float value)
{
    maxGreenTRQ = value;
}

void  TRQGauge::setTRQYellowArcStart(float value)
{
    minYellowTRQ = value;
}

void  TRQGauge::setTRQYellowArcEnd(float value)
{
    maxYellowTRQ = value;
}

void  TRQGauge::setTRQRedline(float value)
{
    redlineTRQ = value;
}

void TRQGauge::setInstrumentBrightness(float value)
{
    float pwmOutput = 0;
    instrumentBrightness = scaleValue(value, 0, 1, 0, 255);
    pwmOutput = sq(instrumentBrightness) / 255.0;  // needed to correct PWM output due to human eye brightness perception
    analogWrite(TFT_BL, pwmOutput);
}

void TRQGauge::setPowerSave(bool enabled)
{
    if (enabled) {
        powerSaveFlag = true;
    } else {
        powerSaveFlag = false;
    }
}

// Scale function
float TRQGauge::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}