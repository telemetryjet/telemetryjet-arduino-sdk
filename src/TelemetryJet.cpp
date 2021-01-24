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

TelemetryJet::TelemetryJet(Stream *transport, unsigned long transmitRate)
  : transport(transport), transmitRate(transmitRate) {
  // Initialize variable-size dimensions array
  dimensions = (DataPoint**) malloc(sizeof(DataPoint*) * dimensionCacheLength);
}

/*
 * StuffData byte stuffs "length" bytes of data
 * at the location pointed to by "ptr", writing
 * the output to the location pointed to by "dst".
 *
 * Returns the length of the encoded data.
 * From https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */
size_t StuffData(const uint8_t *ptr, size_t length, uint8_t *dst) {
  uint8_t *start = dst;
  uint8_t *code_ptr = dst++;

  *code_ptr = 1;
  while (length--) {
      if (*ptr) {
          *dst++ = *ptr++;
          *code_ptr += 1;
      } else {
          code_ptr = dst++;
          *code_ptr = 1;
          ptr++;
      }

      if (*code_ptr == 0xFF && length > 0) {
          code_ptr = dst++;
          *code_ptr = 1;
      }
  }
  *dst++ = 0;

  return dst - start;
}

/*
 * UnStuffData decodes "length" bytes of data at
 * the location pointed to by "ptr", writing the
 * output to the location pointed to by "dst".
 *
 * Returns the length of the decoded data
 * (which is guaranteed to be <= length).
 * From https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */
size_t UnStuffData(const uint8_t *ptr, size_t length, uint8_t *dst) {
  const uint8_t *start = dst, *end = ptr + length;
  uint8_t code = 0xFF, copy = 0;
  bool flag = true;

  for (; ptr < end; copy--) {
      if (copy != 0) {
          flag = false;
          *dst++ = *ptr++;
      } else {
          if (code != 0xFF) {
              flag = true;
              *dst++ = 0;
          }
          copy = code = *ptr++;
          if (code == 0) {
              break;
          }
      }
  }

  if (flag) {
      --dst;
  }

  return dst - start;
}

void TelemetryJet::update() {
  if (!isInitialized) {
    if (hasBinaryWarningMessage && !isTextMode) {
      transport->println(F("Started streaming data in Binary mode. This data is not human-readable."));
      transport->println(F("For usage information, please see https://docs.telemetryjet.com/."));
    }
    isInitialized = true; 
  }

  if (isTextMode) {
    // Text mode
    // Don't read inputs; just log as text output to the serial stream
    // Useful for debugging purposes
    while (transport->available() > 0) {
      uint8_t inByte = transport->read();
    }

    if (millis() - lastSent >= transmitRate && numDimensions > 0) {
      bool wroteValue = false;
      bool hasValue = !isDeltaMode;
      for (uint16_t i = 0; i < numDimensions; i++) {
        updateHasValue(i);
        if (dimensions[i]->hasNewTransmitValue) {
          dimensions[i]->hasNewTransmitValue = false;
          hasValue = true;
        }
      }
      if (hasValue) {
        for (uint16_t i = 0; i < numDimensions; i++) {
          if (dimensions[i]->hasValue) {
            switch (dimensions[i]->type) {
              case DataPointType::BOOLEAN: {
                transport->print((unsigned int)(dimensions[i]->value.v_bool));
                break;
              }
              case DataPointType::UINT8: {
                transport->print((unsigned int)(dimensions[i]->value.v_uint8));
                break;
              }
              case DataPointType::UINT16: {
                transport->print((unsigned int)(dimensions[i]->value.v_uint16));
                break;
              }
              case DataPointType::UINT32: {
                transport->print((unsigned long)(dimensions[i]->value.v_uint32));
                break;
              }
              case DataPointType::UINT64: {
                transport->print((unsigned long)(dimensions[i]->value.v_uint64));
                break;
              }
              case DataPointType::INT8: {
                transport->print((int)(dimensions[i]->value.v_int8));
                break;
              }
              case DataPointType::INT16: {
                transport->print((int)(dimensions[i]->value.v_int16));
                break;
              }
              case DataPointType::INT32: {
                transport->print((long)(dimensions[i]->value.v_int32));
                break;
              }
              case DataPointType::INT64: {
                transport->print((long)(dimensions[i]->value.v_int64));
                break;
              }
              case DataPointType::FLOAT32: {
                transport->print((float)(dimensions[i]->value.v_float32));
                break;
              }
              default: {
                break;
              }
            }
          } else {
            transport->write('0');
          }
          transport->write(' ');
        }
        transport->write('\n');
      }
      lastSent = millis();
    }
  } else {
    // Binary mode
    while (transport->available() > 0) {
      uint8_t inByte = transport->read();
      if (rxIndex >= 32) {
        rxIndex = 0;
      }
      rxBuffer[rxIndex++] = inByte;

      // 0x0 pads the end of a packet
      // Reset the buffer and parse if possible
      if (inByte == 0x0) {
        if (rxIndex > 6) {
          // If we see 0x0 and have contents in the buffer, read packet
          // Minimum length of a packet is 5 bytes:
          // - Checksum (1 byte)
          // - Checksum correction byte (1 byte)
          // - COBS header byte (1 byte)
          // - Key (1+ byte)
          // - Type (1+ byte)
          // - Value (1+ byte)
          // - Packet boundary (0x0, 1 byte)
          
          uint8_t checksumValue = (uint8_t)rxBuffer[0];
          uint8_t paddingByteValue = (uint8_t)rxBuffer[1];

          // 1 - Validate checksum
          uint8_t checksum = 0;
          for (uint16_t bufferIdx = 0; bufferIdx < rxIndex; bufferIdx++) {
            checksum += (uint8_t)rxBuffer[bufferIdx];
          }
          // Get padding/flag byte

          if (checksum == 0xFF) {
            // Expand COBS encoded binary string
            // Offset the array by the two checksum bytes that are not contained in the cobs encoding
            size_t packetLength = UnStuffData(rxBuffer + 2, rxIndex - 2, tempBuffer);

            // Process messagepack structure
            mpack_reader_t reader;
            mpack_reader_init_data(&reader, tempBuffer, packetLength);

            uint16_t key = mpack_expect_u16(&reader);
            uint8_t type = mpack_expect_u8(&reader);
            DataPointValue value;

            switch ((DataPointType)type) {
              case DataPointType::BOOLEAN: {
                value.v_bool = mpack_expect_bool(&reader);
                break;
              }
              case DataPointType::UINT8: {
                value.v_uint8 = mpack_expect_u8(&reader);
                break;
              }
              case DataPointType::UINT16: {
                value.v_uint16 = mpack_expect_u16(&reader);
                break;
              }
              case DataPointType::UINT32: {
                value.v_uint32 = mpack_expect_u32(&reader);
                break;
              }
              case DataPointType::UINT64: {
                value.v_uint64 = mpack_expect_u64(&reader);
                break;
              }
              case DataPointType::INT8: {
                value.v_int8 = mpack_expect_i8(&reader);
                break;
              }
              case DataPointType::INT16: {
                value.v_int16 = mpack_expect_i16(&reader);
                break;
              }
              case DataPointType::INT32: {
                value.v_int32 = mpack_expect_i32(&reader);
                break;
              }
              case DataPointType::INT64: {
                value.v_int64 = mpack_expect_i64(&reader);
                break;
              }
              case DataPointType::FLOAT32: {
                value.v_float32 = mpack_expect_float(&reader);
                break;
              }
            }

            if (mpack_reader_destroy(&reader) == mpack_ok) {
              // Write packet values as a data point
              // Find dimension with key matching from the data
              if (type < (uint8_t)DataPointType::NUM_TYPES) {
                for (uint16_t i = 0; i < numDimensions; i++) {
                  if (dimensions[i]->key = key) {
                    dimensions[i]->value = value;
                    dimensions[i]->type = (DataPointType)type;
                    dimensions[i]->hasValue = true;
                    dimensions[i]->hasNewTransmitValue = false;
                    dimensions[i]->hasNewReceivedValue = true;
                    dimensions[i]->lastTimestamp = millis();
                    break;
                  }
                }
              }


            } else {
              numDroppedRxPackets++;
            }
          } else {
            numDroppedRxPackets++;
          }
        }
        rxIndex = 0;
      }
    }
    if (millis() - lastSent >= transmitRate && numDimensions > 0) {
      mpack_writer_t writer;
      size_t packetLength;
      for (uint16_t i = 0; i < numDimensions; i++) {
        updateHasValue(i);
        if (dimensions[i]->hasValue && (dimensions[i]->hasNewTransmitValue || !isDeltaMode)) {
          dimensions[i]->hasNewTransmitValue = false;
          mpack_writer_init(&writer, tempBuffer, 32);

          // Write key and type headers
          mpack_write_u16(&writer, (uint16_t)dimensions[i]->key);
          mpack_write_u8(&writer, (uint8_t)dimensions[i]->type);

          // Write data
          switch (dimensions[i]->type) {
            case DataPointType::BOOLEAN: {
              mpack_write_bool(&writer, dimensions[i]->value.v_bool);
              break;
            }
            case DataPointType::UINT8: {
              mpack_write_u8(&writer, dimensions[i]->value.v_uint8);
              break;
            }
            case DataPointType::UINT16: {
              mpack_write_u16(&writer, dimensions[i]->value.v_uint16);
              break;
            }
            case DataPointType::UINT32: {
              mpack_write_u32(&writer, dimensions[i]->value.v_uint32);
              break;
            }
            case DataPointType::UINT64: {
              mpack_write_u64(&writer, dimensions[i]->value.v_uint64);
              break;
            }
            case DataPointType::INT8: {
              mpack_write_i8(&writer, dimensions[i]->value.v_int8);
              break;
            }
            case DataPointType::INT16: {
              mpack_write_i16(&writer, dimensions[i]->value.v_int16);
              break;
            }
            case DataPointType::INT32: {
              mpack_write_i32(&writer, dimensions[i]->value.v_int32);
              break;
            }
            case DataPointType::INT64: {
              mpack_write_i64(&writer, dimensions[i]->value.v_int64);
              break;
            }
            case DataPointType::FLOAT32: {
              mpack_write_float(&writer, dimensions[i]->value.v_float32);
              break;
            }
          }

          mpack_writer_destroy(&writer);
          packetLength = mpack_writer_buffer_used(&writer);

          // Use COBS (Consistent Overhead Byte Stuffing)
          // https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
          // to replace all 0x0 bytes in the packet.
          // This way, we can use 0x0 as a packet frame marker. 
          packetLength = StuffData(tempBuffer, packetLength, txBuffer);

          // Compute checksum and add to front of the packet
          // We never want the checksum to == 0,
          // because that would complicate the COBS & packet frame marker logic.
          // If the checksum is going to be 0, add a single bit so that it won't be.
          uint8_t checksum = 0;
          for (uint16_t bufferIdx = 0; bufferIdx < packetLength; bufferIdx++) {
            checksum += (uint8_t)txBuffer[bufferIdx];
          }
          checksum = 0xFF - (checksum + 0x01);

          uint8_t checksumCorrectionByte = 0x01;
          if (checksum == 0x0) {
            // Increment byte in the front of the packet to correct the checksum
            // If the checksum was previously 0x0 (0), it will now be 0xFF (255).
            checksumCorrectionByte += 1;
            checksum = 0xFF; 
          }

          // Write checksum and checksum correction byte
          transport->write((uint8_t)checksum);
          transport->write((uint8_t)checksumCorrectionByte);

          // Write buffer
          for (uint16_t bufferIdx = 0; bufferIdx < packetLength; bufferIdx++) {
            transport->write((uint8_t)txBuffer[bufferIdx]);
          }
          numTxPackets++;
        }
      }
      lastSent = millis();
    }
  }
}

Dimension TelemetryJet::createDimension(uint16_t key, uint32_t timeoutAge = 0) {
  // Resize dimension array if it is full
  if (numDimensions >= dimensionCacheLength) {
    DataPoint** newDimensionArray = (DataPoint**) malloc(sizeof(DataPoint*) * (dimensionCacheLength + 8));
    // Copy from old cache into new cache
    for (int i = 0; i < numDimensions; i++) {
      newDimensionArray[i] = dimensions[i];
    }
    free(dimensions);
    dimensions = newDimensionArray;
    dimensionCacheLength = dimensionCacheLength + 8;
  }

  uint16_t dimensionId = numDimensions++;
  // Create a data point for the new dimension and add to the cache array
  DataPoint* newDataPoint = (DataPoint*) malloc(sizeof(DataPoint));
  dimensions[dimensionId] = newDataPoint;
  dimensions[dimensionId]->key = key;
  dimensions[dimensionId]->type = DataPointType::FLOAT32;
  dimensions[dimensionId]->value.v_float32 = 0.0;
  dimensions[dimensionId]->hasValue = false;
  dimensions[dimensionId]->hasNewReceivedValue = false;
  dimensions[dimensionId]->hasNewTransmitValue = false;
  if (timeoutAge > 0) {
    dimensions[dimensionId]->hasTimeout = true;
    dimensions[dimensionId]->timeoutInterval = timeoutAge;
  } else {
    dimensions[dimensionId]->hasTimeout = false;
    dimensions[dimensionId]->timeoutInterval = 0;
  }
  dimensions[dimensionId]->lastTimestamp = 0;
  return Dimension(dimensionId, this);
}

void Dimension::setBool(bool value) {
  _parent->dimensions[_id]->value.v_bool = value;
  _parent->dimensions[_id]->type = DataPointType::BOOLEAN;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setUInt8(uint8_t value) {
  _parent->dimensions[_id]->value.v_uint8 = value;
  _parent->dimensions[_id]->type = DataPointType::UINT8;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setUInt16(uint16_t value) {
  _parent->dimensions[_id]->value.v_uint16 = value;
  _parent->dimensions[_id]->type = DataPointType::UINT16;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setUInt32(uint32_t value) {
  
  _parent->dimensions[_id]->value.v_uint32 = value;
  _parent->dimensions[_id]->type = DataPointType::UINT32;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setUInt64(uint64_t value) {
  _parent->dimensions[_id]->value.v_uint64 = value;
  _parent->dimensions[_id]->type = DataPointType::UINT64;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setInt8(int8_t value) {
  _parent->dimensions[_id]->value.v_int8 = value;
  _parent->dimensions[_id]->type = DataPointType::INT8;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setInt16(int16_t value) {
  _parent->dimensions[_id]->value.v_int16 = value;
  _parent->dimensions[_id]->type = DataPointType::INT16;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setInt32(int32_t value) {
  _parent->dimensions[_id]->value.v_int32 = value;
  _parent->dimensions[_id]->type = DataPointType::INT32;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setInt64(int64_t value) {
  _parent->dimensions[_id]->value.v_int64 = value;
  _parent->dimensions[_id]->type = DataPointType::INT64;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setFloat32(float value) {
  _parent->dimensions[_id]->value.v_float32 = value;
  _parent->dimensions[_id]->type = DataPointType::FLOAT32;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewReceivedValue = false;
  _parent->dimensions[_id]->hasNewTransmitValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

bool Dimension::getBool(bool defaultValue = false) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::BOOLEAN) {
    return _parent->dimensions[_id]->value.v_bool;
  } else {
    return defaultValue;
  }
}

uint8_t Dimension::getUInt8(uint8_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::UINT8) {
    return _parent->dimensions[_id]->value.v_uint8;
  } else {
    return (uint8_t)getBool(defaultValue);
  }
}

uint16_t Dimension::getUInt16(uint16_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::UINT16) {
    return _parent->dimensions[_id]->value.v_uint16;
  } else {
    return (uint16_t)getUInt8(defaultValue);
  }
}

uint32_t Dimension::getUInt32(uint32_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::UINT32) {
    return _parent->dimensions[_id]->value.v_uint32;
  } else {
    return (uint32_t)getUInt16(defaultValue);
  }
}

uint64_t Dimension::getUInt64(uint64_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::UINT64) {
    return _parent->dimensions[_id]->value.v_uint64;
  } else {
    return (uint64_t)getUInt32(defaultValue);
  }
}

int8_t Dimension::getInt8(int8_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::INT8) {
    return _parent->dimensions[_id]->value.v_int8;
  } else {
    return defaultValue;
  }
}

int16_t Dimension::getInt16(int16_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::INT16) {
    return _parent->dimensions[_id]->value.v_int16;
  } else {
    return (int16_t)getInt8(defaultValue);
  }
}

int32_t Dimension::getInt32(int32_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::INT32) {
    return _parent->dimensions[_id]->value.v_int32;
  } else {
    return (int32_t)getInt16(defaultValue);
  }
}

int64_t Dimension::getInt64(int64_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::INT64) {
    return _parent->dimensions[_id]->value.v_int64;
  } else {
    return (int64_t)getInt32(defaultValue);
  }
}

float Dimension::getFloat32(float defaultValue = 0.0) {
  if (!hasValue()) {
    return defaultValue;
  }

  if (_parent->dimensions[_id]->type == DataPointType::FLOAT32) {
    return _parent->dimensions[_id]->value.v_float32;
  } else {
    return defaultValue;
  }
}

bool Dimension::hasBool(bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::BOOLEAN) {
    return true;
  }
  if (!exact) {
    return false;
  }
}

bool  Dimension::hasUInt8  (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::UINT8) {
    return true;
  }
  if (!exact) {
    return hasBool();
  }
}

bool Dimension::hasUInt16 (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::UINT16) {
    return true;
  }
  if (!exact) {
    return hasUInt8();
  }
}

bool Dimension::hasUInt32 (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::UINT32) {
    return true;
  }
  if (!exact) {
    return hasUInt16();
  }
}

bool Dimension::hasUInt64 (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::UINT64) {
    return true;
  }
  if (!exact) {
    return hasUInt32();
  }
}

bool Dimension::hasInt8   (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::INT8) {
    return true;
  }
  if (!exact) {
    return false;
  }
}

bool Dimension::hasInt16  (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::INT16) {
    return true;
  }
  if (!exact) {
    return hasInt8();
  }
}

bool Dimension::hasInt32  (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::INT32) {
    return true;
  }
  if (!exact) {
    return hasInt16();
  }
}

bool Dimension::hasInt64  (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::INT64) {
    return true;
  }
  if (!exact) {
    return hasInt32();
  }
}

bool Dimension::hasFloat32(bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::FLOAT32) {
    return true;
  }
  if (!exact) {
    return false;
  }
}

DataPointType Dimension::getType() {
  return _parent->dimensions[_id]->type;
}

void Dimension::clearValue() {
  _parent->dimensions[_id]->hasValue = false;
}

// Check if a value is present, and check/update timeout at the same time
bool Dimension::hasValue() {
  if (!(_parent->dimensions[_id]->hasValue)) {
    return false;
  }
  if (_parent->dimensions[_id]->hasTimeout && ((millis() - _parent->dimensions[_id]->lastTimestamp) > _parent->dimensions[_id]->timeoutInterval)) {
    _parent->dimensions[_id]->hasValue = false;
    return false;
  }
  return true;
}

void TelemetryJet::updateHasValue(int id) {
  if (dimensions[id]->hasTimeout && ((millis() - dimensions[id]->lastTimestamp) > dimensions[id]->timeoutInterval)) {
    dimensions[id]->hasValue = false;
  }
}

int32_t Dimension::getTimeoutAge() {
  return _parent->dimensions[_id]->timeoutInterval;
}

int32_t Dimension::getCurrentAge() {
  return (millis() - _parent->dimensions[_id]->lastTimestamp);
}

void Dimension::setTimeoutAge(uint32_t timeoutAge = 0) {
  if (timeoutAge > 0) {
    _parent->dimensions[_id]->hasTimeout = true;
    _parent->dimensions[_id]->timeoutInterval = timeoutAge;
  } else {
    _parent->dimensions[_id]->hasTimeout = false;
    _parent->dimensions[_id]->timeoutInterval = 0;
  }
}

bool Dimension::hasNewValue() {
  if (_parent->dimensions[_id]->hasNewReceivedValue) {
    _parent->dimensions[_id]->hasNewReceivedValue = false;
    return true;
  }
  return false;
}