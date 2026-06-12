#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include "ui.h"
#include "RC_Input.h"
#include "RC_Radio.h"
#include "RC_Shared.h"

Preferences preferences;

static const uint32_t KEY_SALT_A = 0xA3F1C7D2;
static const uint32_t KEY_SALT_B = 0x5E9B2F84;

String generateDeviceKey() {
    uint64_t mac = ESP.getEfuseMac();

    uint8_t b[6];
    b[0] = (mac >>  0) & 0xFF;
    b[1] = (mac >>  8) & 0xFF;
    b[2] = (mac >> 16) & 0xFF;
    b[3] = (mac >> 24) & 0xFF;
    b[4] = (mac >> 32) & 0xFF;
    b[5] = (mac >> 40) & 0xFF;

    uint16_t seg1 = (((uint16_t)b[0] << 8) | b[1]) ^ (uint16_t)(KEY_SALT_A >> 16);
    uint16_t seg2 = (((uint16_t)b[2] << 8) | b[3]) ^ (uint16_t)(KEY_SALT_A & 0xFFFF);
    uint16_t seg3 = (((uint16_t)b[4] << 8) | b[5]) ^ (uint16_t)(KEY_SALT_B >> 16);

    char buf[16];
    snprintf(buf, sizeof(buf), "%04X-%04X-%04X", seg1, seg2, seg3);
    return String(buf);
}

String getDeviceID() {
    uint64_t mac = ESP.getEfuseMac();
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             (uint8_t)(mac >> 40), (uint8_t)(mac >> 32),
             (uint8_t)(mac >> 24), (uint8_t)(mac >> 16),
             (uint8_t)(mac >>  8), (uint8_t)(mac >>  0));
    return String(buf);
}

void startCriticalProcess();
void stopCriticalProcess();
void setScreenState(bool on);
void finishCalibration();
void checkActivation();

#define ENCRYPTION_KEY 0xAB

static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

const unsigned long timeoutDuration = 10000;
unsigned long lastActivityTime  = 0;
unsigned long lastDebugPrint    = 0;
bool isProcessRunning = false;
bool isScreenOn       = true;

int calState    = 0;
int calProgress = 0;
unsigned long lastBarUpdate = 0;

#define RADIO_SCK  25
#define RADIO_MISO 26
#define RADIO_MOSI 27
#define RADIO_CSN  14
#define RADIO_CE   33

#define PIN_JOY_RIGHT_X 35
#define PIN_JOY_RIGHT_Y 39
#define PIN_JOY_LEFT_X  34
#define PIN_JOY_LEFT_Y  36

RC_Radio radioSystem(RADIO_CE, RADIO_CSN, RADIO_SCK, RADIO_MISO, RADIO_MOSI);

RC_Input rightStick(PIN_JOY_RIGHT_X, PIN_JOY_RIGHT_Y);
RC_Input leftStick(PIN_JOY_LEFT_X, PIN_JOY_LEFT_Y);

uint8_t myUniqueID[5];
JoystickData txData;
TelemetryData rxData;
unsigned long lastRadioTx = 0;

int8_t currentRFPower = 2;

uint16_t calData[5] = {383, 3397, 306, 3352, 4};
TFT_eSPI tft = TFT_eSPI();

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

void checkActivation() {
    preferences.begin("flight-link", false);

    if (preferences.getBool("active", false)) {
        Serial.println("SYSTEM: Device is ACTIVATED.");
        preferences.end();
        return;
    }

    String deviceKey = generateDeviceKey();
    String deviceID  = getDeviceID();

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("ACTIVATION REQUIRED");

    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 55);
    tft.print("Device MAC Address:");
    tft.setTextSize(2);
    tft.setCursor(10, 70);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(deviceID);

    tft.setTextColor(0x8410, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 110);
    tft.print("Enter your activation key");
    tft.setCursor(10, 124);
    tft.print("via Serial Monitor to activate.");

    tft.drawFastHLine(10, 145, 300, 0x4208);
    tft.setTextColor(0x4208, TFT_BLACK);
    tft.setCursor(10, 152);
    tft.print("Use your key generator tool with");
    tft.setCursor(10, 164);
    tft.print("the MAC above to get your key.");

    Serial.println();
    Serial.println("################################################");
    Serial.println("##          ACTIVATION REQUIRED               ##");
    Serial.println("################################################");
    Serial.print("  Device MAC : "); Serial.println(deviceID);
    Serial.println("  Use your key generator tool to get the key.");
    Serial.println("  Send the activation key via Serial.");
    Serial.println("################################################");
    Serial.println();

    while (true) {
        if (Serial.available() > 0) {
            String input = Serial.readString();
            input.trim();
            input.toUpperCase();

            if (input == deviceKey) {
                Serial.println(">> KEY VALID. ACTIVATING...");
                preferences.putBool("active", true);

                tft.fillScreen(0x0460);
                tft.setTextColor(TFT_WHITE, 0x0460);
                tft.setTextSize(2);
                tft.setCursor(55, 100);
                tft.print("ACTIVATED!");
                tft.setTextSize(1);
                tft.setCursor(30, 130);
                tft.print("Restarting in 3 seconds...");
                delay(3000);
                ESP.restart();
            } else {
                Serial.println(">> INVALID KEY. Try again.");
            }
        }
        delay(50);
    }
    preferences.end();
}

extern "C" void setEncryptionState(bool enable) {
    radioSystem.setEncryption(enable, ENCRYPTION_KEY);
    Serial.print("Encryption Changed: ");
    Serial.println(enable ? "ON" : "OFF");
}

extern "C" void startBinding() {
    startCriticalProcess();
    Serial.println("Initiating Bind Sequence...");
    radioSystem.broadcastBind(myUniqueID);
    stopCriticalProcess();
}

extern "C" void startCalibration() {
    Serial.println("Starting Calibration...");
    rightStick.calibrateStep1_Center();
    leftStick.calibrateStep1_Center();
    calProgress = 0;
    calState    = 1;
    if (ui_calB) lv_bar_set_value(ui_calB, 0, LV_ANIM_OFF);
}

extern "C" void setSensitivity(int val) {
    float s = val / 50.0f;
    rightStick.setSensitivity(s);
    leftStick.setSensitivity(s);
    Serial.print("Sensitivity set: ");
    Serial.println(s, 2);
}

extern "C" void setRFPower(int level) {
    currentRFPower = (int8_t)level;
    const char * labels[] = {"LOW", "MED", "HIGH"};
    if (level < 0) level = 0;
    if (level > 2) level = 2;
    if (ui_powerL)      lv_label_set_text(ui_powerL, labels[level]);
    if (ui_powerModeL)  lv_label_set_text(ui_powerModeL, labels[level]);
    Serial.print("RF Power: ");
    Serial.println(labels[level]);
}

extern "C" void setProfile(uint8_t p) {
    txData.profile = p;
    Serial.print("Profile set: ");
    Serial.println(p);
}

extern "C" void setActiveRX(uint8_t rx) {
    Serial.print("Active RX changed to: RX ");
    Serial.println(rx + 1);
}

void finishCalibration() {
    calState = 0;
}

void setScreenState(bool on) {
    if (on) {
        digitalWrite(TFT_BL, HIGH);
        isScreenOn = true;
    } else {
        digitalWrite(TFT_BL, LOW);
        isScreenOn = false;
    }
}

void startCriticalProcess() {
    isProcessRunning = true;
    setScreenState(true);
    lastActivityTime = millis();
}

void stopCriticalProcess() {
    isProcessRunning = false;
    lastActivityTime = millis();
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    tft.getTouchRaw(&touchX, &touchY);
    uint16_t touchZ = tft.getTouchRawZ();

    if (touchZ < 200) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        if (!isScreenOn) {
            setScreenState(true);
            lastActivityTime = millis();
            data->state = LV_INDEV_STATE_REL;
            return;
        }
        lastActivityTime = millis();
        data->state = LV_INDEV_STATE_PR;

        data->point.x = map(constrain(touchX, 300, 3600), 300, 3600, 320, 0);
        data->point.y = map(constrain(touchY, 250, 3750), 250, 3750, 0, 240);
    }
}

static void updateStatusLabels() {
    if (!ui_signalStrL && !ui_signalStrL1) return;

    char sigBuf[16];
    snprintf(sigBuf, sizeof(sigBuf), "%d", (int)rxData.rssi);
    if (ui_signalStrL)  lv_label_set_text(ui_signalStrL,  sigBuf);
    if (ui_signalStrL1) lv_label_set_text(ui_signalStrL1, sigBuf);
}

static void updateTxIdLabel() {
    if (!ui_idL) return;
    char idBuf[24];
    snprintf(idBuf, sizeof(idBuf), "%02X%02X-%02X%02X-%02X",
             myUniqueID[0], myUniqueID[1],
             myUniqueID[2], myUniqueID[3],
             myUniqueID[4]);
    lv_label_set_text(ui_idL, idBuf);
}

void setup() {
    Serial.begin(115200);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.setRotation(3);
    tft.setTouch(calData);

    checkActivation();

    rightStick.begin();
    leftStick.begin();
    lastActivityTime = millis();

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = screenWidth;
    disp_drv.ver_res  = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    ui_init();

    Serial.println("Initializing Radio...");
    if (radioSystem.begin()) {
        Serial.println("Radio Hardware OK (HSPI Custom)");
    } else {
        Serial.println("Radio Hardware FAILED");
    }

    radioSystem.setTransmissionMode(MODE_WIFI_ONLY);

    radioSystem.generateUniqueAddress(myUniqueID);
    radioSystem.setTxAddress(myUniqueID);

    updateTxIdLabel();

    Serial.println("Setup done");
}

void loop() {
    lv_timer_handler();
    delay(5);
    lv_tick_inc(5);

    if (!isProcessRunning && isScreenOn && (millis() - lastActivityTime > timeoutDuration)) {
        setScreenState(false);
    }

    if (millis() - lastDebugPrint > 200) {
        lastDebugPrint = millis();
        int rawLX = analogRead(PIN_JOY_LEFT_X);
        int rawLY = analogRead(PIN_JOY_LEFT_Y);
        int rawRX = analogRead(PIN_JOY_RIGHT_X);
        int rawRY = analogRead(PIN_JOY_RIGHT_Y);

        Serial.print("LEFT RAW X:"); Serial.print(rawLX); Serial.print(" Y:"); Serial.print(rawLY);
        Serial.print(" | RIGHT RAW X:"); Serial.print(rawRX); Serial.print(" Y:"); Serial.println(rawRY);
    }

    if (calState == 1) {
        rightStick.calibrateStep2_UpdateLimits();
        leftStick.calibrateStep2_UpdateLimits();

        if (millis() - lastBarUpdate > 50) {
            lastBarUpdate = millis();
            calProgress++;
            if (calProgress > 100) {
                calProgress = 100;
                finishCalibration();
            }
            if (ui_calB) lv_bar_set_value(ui_calB, calProgress, LV_ANIM_OFF);
        }
    }

    if (millis() - lastRadioTx > 20) {
        lastRadioTx = millis();

        txData.val_x_left  = leftStick.getMappedX();
        txData.val_y_left  = leftStick.getMappedY();
        txData.val_x_right = rightStick.getMappedX();
        txData.val_y_right = rightStick.getMappedY();

        radioSystem.sendData(txData, rxData);

        updateStatusLabels();
    }
}