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

/*
DataPointType
Enumerates all data point value types.
*/
enum class DataPointType : int {
    BOOLEAN,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    INT8,
    INT16,
    INT32,
    INT64,
    FLOAT32,
    FLOAT64
};

/*
DataPointValue
Container for typed data point values.
*/
union DataPointValue {
  bool v_bool;
  uint8_t v_uint8;
  uint16_t v_uint16;
  uint32_t v_uint32;
  uint64_t v_uint64;
  int8_t v_int8;
  int16_t v_int16;
  int32_t v_int32;
  int64_t v_int64;
  float v_float32;
  double v_float64;
};

/*
DataPoint
A single point of data for a dimension
Contains a value, value type, and a timestamp
*/
struct DataPoint {
  uint16_t key;
  DataPointType type;
  DataPointValue value;
  bool hasValue = false;
  bool hasNewValue = false;
  bool hasTimeout = false;
  uint32_t timeoutInterval = 0;
  uint32_t lastTimestamp = 0;
};

class TelemetryJet;

/*
Dimension
This wrapper class holds functions for reading/writing data points with a specific key
Doesn't hold any data, this allows the Dimension to be passed by value instead of by a pointer
A lot of Arduino libraries abstract away pointer operations; this simplifies access for new programmers.
*/
class Dimension {
 private:
  uint16_t _id;
  TelemetryJet* _parent;
  Dimension(uint16_t id, TelemetryJet* parent) : _id(id), _parent(parent) {};
 public:
  // Write a typed value to this dimension
  // Setting a value will record the value, type, and timestamp.
  // The value will be sent on the next update interval tick in update()
  void setBool   (bool value);
  void setUInt8  (uint8_t value);
  void setUInt16 (uint16_t value);
  void setUInt32 (uint32_t value);
  void setUInt64 (uint64_t value);
  void setInt8   (int8_t value);
  void setInt16  (int16_t value);
  void setInt32  (int32_t value);
  void setInt64  (int64_t value);
  void setFloat32(float value);
  void setFloat64(double value);
  
  // Get a typed value from this dimension
  // If a value is not available, returns the default
  // ---- Note: ----
  // Getters convert values UP to a larger type if requested and possible.
  // 
  // For example:
  // - Stored value: int8 -> getInt16 returns a converted value.
  // - Stored value: uint32 -> getUInt64 returns a converted value.
  // - Stored value: float64 -> getFloat32 returns DEFAULT (converting DOWN is not supported.)
  //
  // Full table (stored type, compatible getters):
  // Boolean family:
  // - bool   -> getBool, getUInt8, getUInt16, getUInt32, getUInt64, getInt8, getInt16, getInt32, getInt64
  // Unsigned int family:
  // - uint8  -> getUInt8,  getUInt16, getUInt32, getUInt64
  // - uint16 -> getUInt16, getUInt32, getUInt64
  // - uint32 -> getUInt32, getUInt64
  // - uint64 -> getUInt64
  // Int family:
  // - int8   -> getInt8,  getInt16, getInt32, getInt64
  // - int16  -> getInt16, getInt32, getInt64
  // - int32  -> getInt32, getInt64
  // - int64  -> getInt64
  // Floating point family:
  // - float32 -> getFloat32, getFloat64
  // - float64 -> getFloat64
  bool     getBool   (bool     defaultValue = false);
  uint8_t  getUInt8  (uint8_t  defaultValue = 0);
  uint16_t getUInt16 (uint16_t defaultValue = 0);
  uint32_t getUInt32 (uint32_t defaultValue = 0);
  uint64_t getUInt64 (uint64_t defaultValue = 0);
  int8_t   getInt8   (int8_t   defaultValue = 0);
  int16_t  getInt16  (int16_t  defaultValue = 0);
  int32_t  getInt32  (int32_t  defaultValue = 0);
  int64_t  getInt64  (int64_t  defaultValue = 0);
  float    getFloat32(float    defaultValue = 0.0);
  double   getFloat64(double   defaultValue = 0.0);
  
  // Value checks: Return whether a value is present
  // 'exact' parameter determines whether we must have a value of exactly this type,
  // or can also have a compatible value as described in the Getters section.
  // Default is false, meaning these functions return true if we have a compatible value.
  bool hasValue();
  bool hasBool   (bool exact = false);
  bool hasUInt8  (bool exact = false);
  bool hasUInt16 (bool exact = false);
  bool hasUInt32 (bool exact = false);
  bool hasUInt64 (bool exact = false);
  bool hasInt8   (bool exact = false);
  bool hasInt16  (bool exact = false);
  bool hasInt32  (bool exact = false);
  bool hasInt64  (bool exact = false);
  bool hasFloat32(bool exact = false);
  bool hasFloat64(bool exact = false);

  // Clear a value if it is present
  void clearValue();

  // Metadata and flags: Value type, timeout age
  DataPointType getType();
  int32_t getTimeoutAge();
  int32_t getCurrentAge();
  void setTimeoutAge(uint32_t timeoutAge = 0);
  bool hasNewValue();

  friend class TelemetryJet;
};

class TelemetryJet {
private:
  Stream* transport;
  bool isInitialized = false;
  bool isTextMode = false;
  bool isDeltaMode = true;
  uint32_t lastSent;
  uint32_t transmitRate;

  // Array of dimension values
  // Stores the latest data point for a dimension with a given ID
  // Starts with a slot of 8 items, and increases in chunks of 8 as more dimensions are created
  DataPoint** dimensions;
  uint16_t numDimensions = 0;
  uint16_t dimensionCacheLength = 8;

  // Allocate fixed input, output, and temporary buffers
  char tempBuffer[32];
  char rxBuffer[32];
  char txBuffer[32];
  uint8_t rxIndex;
  uint8_t txIndex;
  uint32_t numDroppedRxPackets;
  uint32_t numRxPackets;
  uint32_t numTxPackets;

  void updateHasValue(int id);
public:
  TelemetryJet(Stream *transport, unsigned long transmitRate);

  // Update all data, handling any new inputs/outputs
  void update();

  // Create a new dimension with a given key
  Dimension createDimension(uint16_t key, uint32_t timeoutAge = 0);

  // Get the number of dimensions
  uint16_t getNumDimensions() {
    return numDimensions;
  }

  void setTextMode(bool textMode = false) {
    isTextMode = textMode;
  }
  void setDeltaMode(bool deltaMode = false) {
    isDeltaMode = deltaMode;
  }

  friend class Dimension;
};

#endif