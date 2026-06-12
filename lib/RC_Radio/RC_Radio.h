#ifndef RC_RADIO_H
#define RC_RADIO_H

#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>       
#include <WiFi.h>       
#include <WiFiUdp.h>    
#include "RC_Shared.h" 

enum RadioMode {
    MODE_NRF_ONLY,
    MODE_WIFI_ONLY,  
    MODE_BOTH
};

class RC_Radio {
private:
    RF24 radio;
    SPIClass * hspi;   
    
    
    WiFiUDP udp;
    const int UDP_PORT = 4210;

    RadioMode currentMode;

    uint8_t _ce, _csn, _sck, _miso, _mosi;
    bool encryptionEnabled;
    uint8_t encryptionKey;
    
    const uint8_t BIND_PIPE[5] = {0x11, 0x22, 0x33, 0x44, 0x55};

public:
    RC_Radio(uint8_t ce, uint8_t csn, uint8_t sck, uint8_t miso, uint8_t mosi);
    
    bool begin();
    void setTransmissionMode(RadioMode mode);

    void setTxAddress(uint8_t* address);
    void generateUniqueAddress(uint8_t* addrOut);
    void setEncryption(bool enable, uint8_t key);
    bool sendData(JoystickData &data, TelemetryData &telemetryDest);
    void broadcastBind(uint8_t* addressToBind);
};

#endif