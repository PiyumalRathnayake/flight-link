#include "RC_Input.h"

RC_Input::RC_Input(uint8_t pX, uint8_t pY) {
    pinX = pX;
    pinY = pY;
    sensitivity = 1.0f;

    cal.minX = 0; cal.maxX = 4095; cal.centerX = 2048;
    cal.minY = 0; cal.maxY = 4095; cal.centerY = 2048;
    cal.deadzone = 50;
    cal.reverseX = false; cal.reverseY = false;
}

void RC_Input::begin() {
    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);
}

void RC_Input::setCalibration(StickCalibration newCal) {
    cal = newCal;
}

StickCalibration RC_Input::getCalibration() {
    return cal;
}

void RC_Input::setSensitivity(float s) {
    if (s < 0.1f) s = 0.1f;
    if (s > 2.0f) s = 2.0f;
    sensitivity = s;
}

float RC_Input::getSensitivity() {
    return sensitivity;
}

void RC_Input::calibrateStep1_Center() {
    long sumX = 0, sumY = 0;
    for(int i = 0; i < 20; i++) {
        sumX += analogRead(pinX);
        sumY += analogRead(pinY);
        delay(2);
    }

    cal.centerX = sumX / 20;
    cal.centerY = sumY / 20;

    cal.minX = cal.centerX;
    cal.maxX = cal.centerX;

    cal.minY = cal.centerY;
    cal.maxY = cal.centerY;
}

void RC_Input::calibrateStep2_UpdateLimits() {
    int rawX = analogRead(pinX);
    int rawY = analogRead(pinY);

    if (rawX < cal.minX) cal.minX = rawX;
    if (rawX > cal.maxX) cal.maxX = rawX;

    if (rawY < cal.minY) cal.minY = rawY;
    if (rawY > cal.maxY) cal.maxY = rawY;
}

int RC_Input::getMappedX() {
    int raw = analogRead(pinX);
    if (abs(raw - (int)cal.centerX) < cal.deadzone) return 500;

    int val = map(raw, cal.minX, cal.maxX, 0, 1000);
    val = constrain(val, 0, 1000);

    int centered = val - 500;
    int scaled = (int)(centered * sensitivity);
    scaled = constrain(scaled, -500, 500);
    val = 500 + scaled;

    return cal.reverseX ? (1000 - val) : val;
}

int RC_Input::getMappedY() {
    int raw = analogRead(pinY);
    if (abs(raw - (int)cal.centerY) < cal.deadzone) return 500;

    int val = map(raw, cal.minY, cal.maxY, 0, 1000);
    val = constrain(val, 0, 1000);

    int centered = val - 500;
    int scaled = (int)(centered * sensitivity);
    scaled = constrain(scaled, -500, 500);
    val = 500 + scaled;

    return cal.reverseY ? (1000 - val) : val;
}