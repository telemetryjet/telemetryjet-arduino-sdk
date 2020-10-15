#ifndef STATELESS_TELEMETRY_NODE_H
#define STATELESS_TELEMETRY_NODE_H

#include "Arduino.h"

struct Packet{
	uint8_t startByte;
  uint8_t data[13];
  uint8_t metaData; //4 bit deviceID | 4 bit packetNum
  uint8_t checksum;
};

enum DeviceID {
	DEVICE_ALLTRAX,
  DEVICE_VESC,
  DEVICE_MOTOR_BOARD,
  DEVICE_BATTERY_BOARD,
  DEVICE_GPS_IMU,
  DEVICE_THROTTLE,
  DEVICE_SOLAR
};

class StatelessTelemetryNode{
	protected:
		uint8_t deviceID;
		uint8_t numPackets;
    Serial_ *_serial;
		Packet* currentPack;
		uint8_t* rxPacket;
		uint8_t rxIndex;
		unsigned long lastSent;
		unsigned long lastRx;
		unsigned long sendInterval;
		virtual void pack(void *p);
    virtual void unpack();
		virtual void dataTimeout();
		void setPacketNum(uint8_t id);
		void sendData();

	public:
		StatelessTelemetryNode(uint8_t deviceID, Serial_ *serialPort, unsigned long sendInterval)
			: deviceID(deviceID), _serial(serialPort), sendInterval(sendInterval){
				setPacketNum(deviceID);
				currentPack = new Packet[numPackets];
				rxPacket = new uint8_t[16];
		};
		void update();
		void begin(long baudrate);
		uint8_t getDeviceID() const { return deviceID; };
		uint8_t getNumPackets() const { return numPackets; };
		unsigned long getlastRx() { return lastRx; };
};

class AlltraxNode : public StatelessTelemetryNode {
  private:
		const uint8_t PACKET_START = 0xF0;
		void pack(void *p);
    void unpack();
		void dataTimeout();
  public:
    uint16_t throt;
	uint16_t enable;
    uint16_t diodeTemp;
    uint16_t inVoltage;
    uint16_t outCurrent;
    uint16_t inCurrent;
    uint8_t dutyCycle;
    uint8_t errorCode;
    AlltraxNode(Serial_ *serialPort, unsigned long sendInterval)
      : StatelessTelemetryNode(DEVICE_ALLTRAX, serialPort, sendInterval){};
};

class VescNode : public StatelessTelemetryNode {
	private:
		const uint8_t PACKET_START = 0xF0;
		void pack(void *p);
		void unpack();
		void dataTimeout();
	public:
		uint16_t throt;
		uint16_t fetTemp;
		uint16_t inVoltage;
		uint16_t outCurrent;
		uint16_t inCurrent;
		uint8_t dutyCycle;
		uint8_t faultCode;
		VescNode(Serial_ *serialPort,unsigned long sendInterval)
			: StatelessTelemetryNode(DEVICE_VESC, serialPort, sendInterval){};
};

class MotorBoardNode : public StatelessTelemetryNode {
	private:
		const uint8_t PACKET_START = 0xF0;
		void pack(void *p);
		void unpack();
		void dataTimeout();
	public:
		float motorTemp;
		uint32_t motorRPM;
		uint32_t propRPM;
		MotorBoardNode(Serial_ *serialPort, unsigned long sendInterval)
			: StatelessTelemetryNode(DEVICE_MOTOR_BOARD, serialPort, sendInterval){};
};

class BatteryNode : public StatelessTelemetryNode {
	private:
		const uint8_t PACKET_START = 0xF0;
		void pack(void *p);
		void unpack();
		void dataTimeout();
	public:
		float batteryVoltage;
		float batteryCurrent;
		float batteryPower;
		float batteryTimeRemaining;
		float batteryConsumedAh;
		float batteryStateOfCharge;
		BatteryNode(Serial_ *serialPort, unsigned long sendInterval)
			: StatelessTelemetryNode(DEVICE_BATTERY_BOARD, serialPort, sendInterval){};
};

class GPSIMUNode : public StatelessTelemetryNode {
	private:
		const uint8_t PACKET_START = 0xF0;
		void pack(void *p);
		void unpack();
		void dataTimeout();
	public:
		float imuPitch;
		float imuRoll;
		float latitude;
		float longitude;
		float speedKnots;
		uint8_t numSatellites;
		uint8_t fix;
		uint8_t heading;
		GPSIMUNode(Serial_ *serialPort, unsigned long sendInterval)
			: StatelessTelemetryNode(DEVICE_GPS_IMU, serialPort, sendInterval){};
};

class ThrottleNode : public StatelessTelemetryNode{
	private:
		const uint8_t PACKET_START = 0xF0;
		void pack(void *p);
		void unpack();
		void dataTimeout();
	public:
		uint16_t throt;
		uint8_t enable;
		uint8_t mode;
		uint8_t config;
		ThrottleNode(Serial_ *serialPort, unsigned long sendInterval)
		 : StatelessTelemetryNode(DEVICE_THROTTLE, serialPort,sendInterval){};
};

class SolarNode : public StatelessTelemetryNode{
 private:
	 const uint8_t PACKET_START = 0xF0;
	 void pack(void *p);
	 void unpack();
	 void dataTimeout();
 public:
	 float outCurrent1;
	 float outCurrent2;
	 float totalCurrent;
	 SolarNode(Serial_ *serialPort, unsigned long sendInterval)
		: StatelessTelemetryNode(DEVICE_SOLAR, serialPort, sendInterval){};
};
#endif
