#ifndef RC_INPUT_H
#define RC_INPUT_H

#include <Arduino.h>

struct StickCalibration {
    uint16_t minX, maxX, centerX;
    uint16_t minY, maxY, centerY;
    uint16_t deadzone;
    bool reverseX, reverseY;
};

class RC_Input {
private:
    uint8_t pinX, pinY;
    StickCalibration cal;
    float sensitivity;

public:
    RC_Input(uint8_t pX, uint8_t pY);
    void begin();

    void setCalibration(StickCalibration newCal);
    StickCalibration getCalibration();

    void setSensitivity(float s);
    float getSensitivity();

    void calibrateStep1_Center();
    void calibrateStep2_UpdateLimits();

    int getMappedX();
    int getMappedY();
};

#endif