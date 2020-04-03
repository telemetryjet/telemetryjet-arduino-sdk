#include "telemetryArduino.h"

uint8_t generateChecksum(struct Packet* p) {
    uint8_t* p8 = (uint8_t*)p;
    uint8_t  s  = 0;
    for (size_t i = 0; i < 8; i++)
        s += p8[i];
    return 0xff - s;
}

uint8_t validateChecksum(uint8_t* p) {
    uint8_t s = 0;
    for (size_t i = 0; i < 8; i++)
        s += p[i];
    return s;
}

void TelemetryNode::sendData() {
    pack((void*)(txPacket));
    for (uint8_t i = 0; i < numPackets; i++) {
        uint8_t* outBytes = (uint8_t*)(&txPacket[i]);
        for (uint8_t j = 0; j < 8; j++) {
            _serial->write(outBytes[j]);
        }
    }
}

void TelemetryNode::begin(long baudrate) {
    _serial->begin(baudrate);
}

void TelemetryNode::update() {
    // write
    if (millis() - lastSent >= sendInterval) {
        sendData();
        lastSent = millis();
    }
}

void TelemetryNode::pack(void* p) {
    Packet* packets      = (Packet*)(p);
    packets[0].startByte = 0xF0;
    packets[0].dataId    = dataId;
    packets[0].data      = (uint32_t)(data);
    packets[0].checksum  = 0x00;
    packets[0].checksum  = generateChecksum(&packets[0]);
}