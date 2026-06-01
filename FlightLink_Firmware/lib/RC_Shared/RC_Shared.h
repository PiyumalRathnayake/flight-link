#ifndef RC_SHARED_H
#define RC_SHARED_H

#include <Arduino.h>

#pragma pack(push, 1)

// Packet Types
#define PACKET_TYPE_CONTROL 1
#define PACKET_TYPE_BIND    2

struct JoystickData {
  uint8_t packetType; 
  bool enc;           
  uint8_t profile;    
  
  int val_x_left;     // Rudder
  int val_y_left;     // Throttle
  int val_x_right;    // Aileron
  int val_y_right;    // Elevator
};

struct BindPacket {
  uint8_t packetType;    
  uint8_t newAddress[5]; 
};

struct TelemetryData {
  int8_t rssi;
};

#pragma pack(pop)

#endif