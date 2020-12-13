/*
TelemetryJet Arduino SDK
Chris Dalke <chrisdalke@gmail.com>

Lightweight communication library for hardware telemetry data. 
Handles bidirectional communication and state management for data points. 
-------------------------------------------------------------------------
Part of the TelemetryJet platform -- Collect, analyze, and share
data from your hardware. Code not required.

Distributed "as is" under the MIT License. See LICENSE.md for details.
*/

#ifndef __TELEMETRYJET_H__
#define __TELEMETRYJET_H__

#include <Arduino.h>

struct DimensionCacheValue {
  const char *readableName;
  float lastValue;
  bool hasNewValue;
  unsigned long lastTimestamp;
};

class TelemetryJet;

class Dimension {
 private:
  uint32_t _id;
  TelemetryJet* _parent;
  Dimension(uint32_t id, TelemetryJet* parent) : _id(id), _parent(parent) {};
 public:
  void set(float value);
  float get();
  friend class TelemetryJet;
};

class TelemetryJet {
private:
  char* messagePackBuffer;
  size_t messagePackBufferSize;
  Stream* _stream;
  bool isInitialized = false;
  unsigned long lastSent;
  unsigned long throttleRate;
  DimensionCacheValue** cacheValues;
  unsigned long numDimensions;
  unsigned long cacheSize;
  DimensionCacheValue** allocateCacheArray(uint32_t size);
  void resizeCacheArray();
  void resizeMessagePackBuffer();
  bool checkExpired(Dimension dimension);
  void set(uint32_t id, float value);
  float get(uint32_t id);
  uint8_t* rxPacket;
  uint8_t rxIndex;
public:
  TelemetryJet(Stream *dataStream, unsigned long throttleRate)
     : _stream(dataStream),
       throttleRate(throttleRate) {
    lastSent = 0;
    numDimensions = 0;
    cacheValues = allocateCacheArray(8);
    cacheSize = 8;
    rxPacket = new uint8_t[32];
    rxIndex = 0;
    messagePackBufferSize = 0;
 };
  void begin();
  void end();
  void update();
  size_t getMessagePackBufferSize() {
    return messagePackBufferSize;
  }
  Dimension* createDimension(const char* key);
  unsigned long getNumDimensions() {
    return numDimensions;
  }
  unsigned long getCacheSize() {
    return cacheSize;
  }
  int getFreeMemory();
  friend class Dimension;
};

#endif