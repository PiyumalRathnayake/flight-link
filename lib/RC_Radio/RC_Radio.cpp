#include "RC_Radio.h"

RC_Radio::RC_Radio(uint8_t ce, uint8_t csn, uint8_t sck, uint8_t miso, uint8_t mosi) 
    : radio(ce, csn) 
{
    _ce = ce; _csn = csn; _sck = sck; _miso = miso; _mosi = mosi;
    hspi = NULL; 
    encryptionEnabled = false; 
    encryptionKey = 0;
    currentMode = MODE_NRF_ONLY; 
}

bool RC_Radio::begin() {
    
    // SSID: "RC_Controller_WiFi"
    // PASS: "12345678" 
    WiFi.softAP("RC_Controller_WiFi", "12345678");
    
    Serial.println("\n--------------------------------------");
    Serial.println("WiFi AP Started!");
    Serial.print("Connect your Pi to: RC_Controller_WiFi");
    Serial.print("\nESP32 IP Address: ");
    Serial.println(WiFi.softAPIP()); 
    Serial.println("--------------------------------------");

    
    hspi = new SPIClass(HSPI);
    hspi->begin(_sck, _miso, _mosi, _csn);

    if (!radio.begin(hspi)) return false;

    radio.setPALevel(RF24_PA_MAX);   
    radio.setDataRate(RF24_250KBPS); 
    radio.setRetries(3, 5);          
    radio.stopListening();
    radio.openWritingPipe(BIND_PIPE); 
    
    return true;
}

void RC_Radio::setTransmissionMode(RadioMode mode) {
    currentMode = mode;
}

void RC_Radio::setTxAddress(uint8_t* address) {
    radio.openWritingPipe(address);
}

void RC_Radio::generateUniqueAddress(uint8_t* addrOut) {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    for(int i=0; i<5; i++) addrOut[i] = mac[i+1];
}

void RC_Radio::setEncryption(bool enable, uint8_t key) {
    encryptionEnabled = enable;
    encryptionKey = key;
}

void RC_Radio::broadcastBind(uint8_t* addressToBind) {
    radio.openWritingPipe(BIND_PIPE);
    radio.stopListening();
    
    BindPacket bPkg;
    bPkg.packetType = PACKET_TYPE_BIND;
    memcpy(bPkg.newAddress, addressToBind, 5);

    for(int i=0; i<50; i++) { 
        radio.write(&bPkg, sizeof(BindPacket));
        delay(5);
    }
    radio.openWritingPipe(addressToBind);
}

bool RC_Radio::sendData(JoystickData &data, TelemetryData &telemetryDest) {
    bool success = false;
    data.packetType = PACKET_TYPE_CONTROL;

    
    if (currentMode == MODE_WIFI_ONLY || currentMode == MODE_BOTH) {
        // Broadcast 192.168.4.255
        // Port 4210
        udp.beginPacket("192.168.4.2", UDP_PORT);
        udp.write((const uint8_t*)&data, sizeof(JoystickData));
        udp.endPacket();

        if (currentMode == MODE_WIFI_ONLY) return true; 
    }

    
    if (currentMode == MODE_NRF_ONLY || currentMode == MODE_BOTH) {
        
        if (encryptionEnabled) {
            data.enc = true;
            uint8_t* ptr = (uint8_t*)&data;
            for (size_t i = 2; i < sizeof(JoystickData); i++) ptr[i] ^= encryptionKey;
        } else {
            data.enc = false;
        }

        radio.stopListening();
        success = radio.write(&data, sizeof(JoystickData));
        
        if (success) {
            radio.startListening();
            unsigned long start = millis();
            while (!radio.available()) {
                if (millis() - start > 20) break; 
            }
            if (radio.available()) {
                radio.read(&telemetryDest, sizeof(TelemetryData));
            }
        }
        
        if (encryptionEnabled) {
             uint8_t* ptr = (uint8_t*)&data;
             for (size_t i = 2; i < sizeof(JoystickData); i++) ptr[i] ^= encryptionKey;
        }
    }

    return success;
}