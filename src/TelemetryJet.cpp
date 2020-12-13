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

#include "TelemetryJet.h"
#include "MessagePack.h"

const char* timestampField = "ts";

void TelemetryJet::begin() {
  isInitialized = true;
}

void TelemetryJet::end() {
  isInitialized = false;
}

void TelemetryJet::update() {
  if (!isInitialized) {
    return;
  }

  // Read in data if any is available
  while(_stream->available() > 0){
    byte inByte = _stream->read();
    if(rxIndex < 32) {
      if (inByte == '\n') {
        rxPacket[rxIndex++] = '\0';
        set(0, atoi((char*)rxPacket));
        rxIndex = 0;
      } else {
        rxPacket[rxIndex++] = inByte;
      }
    } else {
      // Ignore until we reach next newline.
      if (inByte == '\n') {
        rxIndex = 0;
      }
    }
  }


  // Once every polling interval, write all data points in the cache as a MessagePack object
  if (millis() - lastSent >= throttleRate && numDimensions > 0) {
    mpack_writer_t writer;
    mpack_writer_init(&writer, messagePackBuffer, messagePackBufferSize);

    uint32_t mapSize = numDimensions + 1; // Item for each dimension, 1 item for timestamp
    mpack_start_map(&writer, mapSize);
    mpack_write_cstr(&writer, timestampField);
    mpack_write_u32(&writer, millis());
    
    for (uint32_t i = 0; i < numDimensions; i++) {
      if (cacheValues[i]->hasNewValue || true) {
        cacheValues[i]->hasNewValue = false;
        mpack_write_cstr(&writer, cacheValues[i]->readableName);
        mpack_write_float(&writer, cacheValues[i]->lastValue);
      }
    }

    size_t bytesUsed = mpack_writer_buffer_used(&writer);
    mpack_error_t error = mpack_writer_destroy(&writer);
    if (error == mpack_ok) {
      // Send messagepack buffer
      _stream->write(messagePackBuffer, bytesUsed);
      _stream->write('\n');
    }

    lastSent = millis();
  }
}

void Dimension::set(float value) {
  _parent->set(_id, value);
}
float Dimension::get() {
  return _parent->get(_id);
}

void TelemetryJet::set(uint32_t id, float value) {
  cacheValues[id]->lastValue = value;
  cacheValues[id]->lastTimestamp = millis();
  cacheValues[id]->hasNewValue = true;
}

float TelemetryJet::get(uint32_t id) {
  return cacheValues[id]->lastValue;
}

Dimension* TelemetryJet::createDimension(const char* key) {
  // Resize cache array it is full
  if (numDimensions >= cacheSize) {
    resizeCacheArray();
  }

  // Create a dimension cache value and add to the cache array
  DimensionCacheValue* newDimensionCacheValue = (DimensionCacheValue*) malloc(sizeof(DimensionCacheValue));
  uint32_t dimensionId = (uint32_t)numDimensions;
  numDimensions++;
  cacheValues[dimensionId] = newDimensionCacheValue;
  cacheValues[dimensionId]->readableName = key;
  cacheValues[dimensionId]->lastValue = 0;
  cacheValues[dimensionId]->lastTimestamp = 0;
  cacheValues[dimensionId]->hasNewValue = false;

  resizeMessagePackBuffer();

  return new Dimension(dimensionId, this);
}

DimensionCacheValue** TelemetryJet::allocateCacheArray(uint32_t size) {
  return (DimensionCacheValue**) malloc(sizeof(DimensionCacheValue*) * size);
}

void TelemetryJet::resizeCacheArray() {
  unsigned long newCacheSize = cacheSize + 8;
  DimensionCacheValue** newCache = allocateCacheArray(newCacheSize);

  // Copy from old cache into new cache
  for (int i = 0; i < numDimensions; i++) {
    newCache[i] = cacheValues[i];
  }

  free(cacheValues);
  cacheValues = newCache;
  cacheSize = newCacheSize;
}

void TelemetryJet::resizeMessagePackBuffer() {
  if (messagePackBufferSize > 0) {
    free(messagePackBuffer);
  }

  // Resize messagepack buffer
  // Size should be computed based on the number of bytes we anticipate in the buffer plus some extra
  messagePackBufferSize = 0;
  messagePackBufferSize += 3;  // 3 bytes for map header
  // Add to the buffer for every dimension, and the timestamp value
  for (int i = 0; i < numDimensions + 1; i++) {
    messagePackBufferSize += 1; // header for string
    messagePackBufferSize += strlen(cacheValues[i]->readableName) + 1; // n bytes based on length of string
    messagePackBufferSize += 5; // 5 bytes for a normal float/int32 and its header
  }
  
  messagePackBufferSize += 8; // Add 8 bytes to end in case we calculated something wrong
  messagePackBuffer = (char *)malloc(sizeof(char) * messagePackBufferSize);
}

extern unsigned int __heap_start;
extern void *__brkval;

/*
 * The free list structure as maintained by the
 * avr-libc memory allocation routines.
 */
struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

/* The head of the free list structure */
extern struct __freelist *__flp;


/* Calculates the size of the free list */
int freeListSize() {
  struct __freelist* current;
  int total = 0;
  for (current = __flp; current; current = current->nx) {
    total += 2; /* Add two bytes for the memory block's header  */
    total += (int) current->sz;
  }
  return total;
}

int TelemetryJet::getFreeMemory() {
  int free_memory;
  if ((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  } else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
    free_memory += freeListSize();
  }
  return free_memory;
}