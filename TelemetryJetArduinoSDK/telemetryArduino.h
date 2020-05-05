#ifndef STATELESS_TELEMETRY_NODE_H
#define STATELESS_TELEMETRY_NODE_H

#include "Arduino.h"

struct Packet {
    uint8_t  startByte;
    uint16_t dataId;
    uint32_t data;
    uint8_t  checksum;
};

// 8 bytes per packet
// 1 byte - start
// 4 bytes - float
// 2 bytes - data id
// 1 byte - checksum

class TelemetryNode {
protected:
    uint8_t         numPackets;
    HardwareSerial* _serial;
    Packet*         txPacket;
    unsigned long   lastSent;
    unsigned long   sendInterval;
    const uint8_t   PACKET_START = 0xF0;
    void            pack(void* p);
    void            sendData();

public:
    uint16_t dataId;
    float    data;
    TelemetryNode(Stream* serialPort, unsigned long sendInterval)
        : _serial(serialPort)
        , sendInterval(sendInterval) {
        numPackets = 1;
        txPacket   = new Packet[numPackets];
    };
    void update();
    void begin(long baudrate);
};
#endif
