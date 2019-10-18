# Telemetry-Node-Stateless-SDK
Stateless version of the telemetry node SDK for Arduino.

## Protocol
Data is sent in 16-byte packets at 115200 baud. Streaming data should be sent
at 4hz. Data can be decimated on the server side so improve performance if
needed but the devices should still send data regularly.

### Byte Breakdown
- Byte 0: Header (0xF0)
- Byte 1-4: Data (1)
- Byte 5-8: Data (2)
- Byte 9-12: Data (3)
- Byte 13: Data (misc)
- Byte 14: packet# (typically 0x00)
- Byte 15: 8-bit Checksum

The device id is unique to the type of device but not to the device itself,
for example a device connected to an Alltrax AXE might have an ID of 0x00.
The last byte of the transmission should be assigned at before it is sent
so that the overflow sum of the previous 15-bytes is 0x00.
(i.e. 0xFF-sumOfPrevFifteen).

### Device IDs
Device IDs are unique to the type of board and are used as the lead byte for
serial transmission. More device IDs will be assigned in future versions as necessary.
Device IDs are enumerated and defined in telemetryNode.h. Current device ID's are listed below:
- DEVICE_ALLTRAX = 0x00 (Alltrax Motor Controller)
- DEVICE_VESC = 0x01 (VESC controller)
- DEVICE_MOTOR_BOARD = 0x02 (Motor board)
- DEVICE_BATTERY_BOARD = 0x03 (Battery board)
- DEVICE_GPS_IMU = 0x04 (GPS/accelerometer board)
- DEVICE_THROTTLE = 0x05 (Throttle board)
- DEVICE_SOLAR = 0x06 (Solar Board)

### Packet Number
For the most part, only one packet of data is required to transmit all the necessary
data from each board and the packet number will be 0x00 but in some circumstances,
more than one transmission may be required. In these cases, the packet number will
be used to identify which data packet is being sent to help with serial parsing.

___

## Device packing
Each of the system boards/nodes will be discussed here with the types of data that will sent
and the packing used to send the data.

### DEVICE_ALLTRAX (0x00)
The following data points will be sent from the Alltrax controller:
#### Controller temp (diodeTemp)
- Description: Internal temperature of the controller in deg C
- Size: 2 bytes _diodeTemp=(diodeTempH<<8)&diodeTempL_
- Encoding: tempC = (diodeTemp-0x0C)(.48828125)

#### Battery Voltage (inVoltage)
- Description: Input voltage to the motor controller
- Size: 2 bytes _inVoltage=(inVoltageH<<8)&inVoltageL_
- Encoding: voltage = inVoltage(.1025)

#### Motor Current (outCurrent)
- Description: Output current to the motor from controller (A)
- Size: 2 bytes _outCurrent=(outCurrentH<<8)&outCurrentL_
- Encoding: 1A/bit

#### Battery Current (inCurrent)
- Description: Current input to the motor controller (A). Calculated from dutyCycle and outCurrent
- Size: 2 bytes _inCurrent=(inCurrentH<<8)&inCurrentL_
- Encoding: 1A/bit

#### Duty Cycle (dutyCycle)
- Description: duty cycle(%) for driving the motor mapped from 0-255. Technically called throttle %
- Size: 1 byte
- Encoding: percent mapped 0-255

#### Error Code (errorCode)
- Description: each bit is a flag for an Alltrax error code.
- Size: 1 byte
- Encoding:
  - Bit 0: Over-throttle
  - Bit 1: Under-temp
  - Bit 2: HPD
  - Bit 3: Over-temp
  - Bit 4: unused
  - Bit 5: Under-voltage
  - Bit 6: Over-voltage
  - Bit 7: controller in boot sequence

#### Alltrax packing
- Byte 0: Header (0xF0)
- Byte 1: diodeTempL
- Byte 2: diodeTempH
- Byte 3: inVoltageL
- Byte 4: inVoltageH
- Byte 5: outCurrentL
- Byte 6: outCurrentH
- Byte 7: inCurrentL
- Byte 8: inCurrentH
- Byte 9: dutyCycle
- Byte 10: errorCode
- Byte 11-13: 0x00 (unused)
- Byte 14: 0x00  (packet #)
- Byte 15: 8-bit checksum

#### Alltrax Unpacking
- Byte 0: Header (0x50)
- Byte 1: throttleL
- Byte 2: throttleH
- Byte 3-13: 0x00 (unused)
- Byte 14: 0x00  (packet #)
- Byte 15: 8-bit checksum

___

### DEVICE_VESC (0x01)
The following data points will be sent from the VESC:
#### Controller Temp (fetTemp)
- Description: average temp of the MOSFETs on the controller in deg C
- Size: 2 bytes _fetTemp=(fetTempH<<8)|fetTempL_
- Encoding: (fetTemp-0x0C)(.48828125)

#### Input Voltage (inVoltage)
- Description: input voltage to controller
- Size: 2 bytes _inVoltage=(inVoltageH<<8)|inVoltageL_
- Encoding: .1025 V/bit

#### Output Current (outCurrent)
- Description: motor coil current (A)
- Size: 2 bytes _outCurrent=(outCurrentH<<8)|outCurrentL_
- Encoding: 1A/bit

#### Input Current (inCurrent)
- Description: input current from batteries
- Size: 2 bytes _inCurrent=(inCurrentH<<8)|inCurrentL_
- Encoding: 1A/Bit

#### Duty Cycle (dutyCycle)
- Description: duty cycle (%) used by the motor
- Size: 1 byte
- Encoding: percent mapped to 0-255

#### Error Code (faultCode)
- Description: each bit is a flag for an VESC fault code.
- Size: 1 byte
- Encoding:
  - Bit 0: OVER_VOLTAGE
  - Bit 1: UNDER_VOLTAGE
  - Bit 2: DRV_FAULT
  - Bit 3: OVER_CURRENT
  - Bit 4: OVER_TEMP_FET
  - Bit 5: OVER_TEMP_MOTOR
  - Bit 6: unused
  - Bit 7: unused

#### VESC packing
- Byte 0: Header (0xF0)
- Byte 1: fetTempL
- Byte 2: fetTempH
- Byte 3: inVoltageL
- Byte 4: inVoltageH
- Byte 5: outCurrentL
- Byte 6: outCurrentH
- Byte 7: inCurrentL
- Byte 8: inCurrentH
- Byte 9: dutyCycle
- Byte 10: faultCode
- Byte 11-13: 0x00 (unused)
- Byte 14: 0x00  (packet #)
- Byte 15: 8-bit checksum

#### VESC Unpacking
- Byte 0: Header (0x50)
- Byte 1: throttleL
- Byte 2: throttleH
- Byte 3-13: 0x00 (unused)
- Byte 14: 0x00  (packet #)
- Byte 15: 8-bit checksum
___

### DEVICE_MOTOR_BOARD(0x02)
Data points to be sent from the custom motor board:

#### Motor Temp (motorTemp)
- Description: Temperature of the motor in deg C (found with thermoresistor)
- Size: 4 bytes
- Encoding: none, standard float (little endian)

#### Motor RPM (motorRPM)
- Description: RPM of the motor shaft (found with Hall-Switch)
- Size: 4 bytes
- Encoding: none, standard int (little endian)

#### Prop RPM (propRPM)
- Description: calculated value of RPM at the prop from motorRPM and gearing
- Size: 4 bytes
- Encoding: none, standard int

#### Motor Board packing
- Byte 0: DeviceID (0x02)
- Bytes 1-4: motorTemp (little endian)
- Bytes 5-8: motorRPM (little endian)
- Bytes 9-12: propRPM (little endian)
- Byte 13: 0x00 (unused)
- Byte 14: 0x00  (packet #)
- Byte 15: 8-bit checksum

___

### DEVICE_GPS_IMU (0x04)
Data points to be sent from the GPS board from Adafruit GPS module:

#### Latitude (lat)
- Description: signed position latitude (e=positive, w=negative)
- Size: 4 bytes
- Encoding: none, standard float (little endian)

#### Longitude (lng)
- Description: signed position longitude (n=positive, s=negative)
- Size: 4 bytes
- Encoding: none, standard float (little endian)

#### Heading Angle (heading)
- Description: mapped angle heading (0,360) -> (0,255)
- Size: 1 byte
- Encoding: (0,360) -> (0,255)

#### Speed (speed)
- Description: Speed of the boat in knots
- Size: 4 bytes
- Encoding: none, standard float

#### Number of Satellites (numSat)
- Description: Number of satellites connected
- Size: 1 byte
- Encoding: none, standard byte (little endian)

#### Epoch Time (time)
- Description: current epoch time in milliseconds
- Size: 4 bytes
- Encoding Standard epoch encoding (little endian)

#### GPS/IMU packing
Packet 0x00:
- Byte 0: Header (0xF0)
- Bytes 1-4: lat
- Bytes 5-8: lng
- Bytes 9-12: time
- Byte 13: numSat
- Byte 14: 0x00 (packet#)
- Byte 15: 8-bit Checksum

Packet 0x01:
- Byte 0: Header (0xF0)
- Bytes 1-4: speed
- Byte 5: heading
- Bytes 6-13: 0x00 (unused)
- Byte 14: 0x01 (packet#)
- Byte 15: 8-bit Checksum

___

### DEVICE_THROTTLE(0x05)
Data points to be sent from the custom motor board:

#### Throttle (throt)
- Description: Throttle percentage
- Size: 2 bytes
- Encoding: throttle percent 1-2^16

#### Throttle packing
- Byte 0: Header (0xF0)
- Bytes 1: throttleL
- Bytes 2: throttleH
- Byte 3-13: 0x00 (unused)
- Byte 14: 0x00  (packet #)
- Byte 15: 8-bit checksum

___

### DEVICE_SOLAR(0x06)
Data points about incoming solar energy

#### Total Output Current
- Description: total output current from solar chargers
- Size: 4 bytes
- Encoding: none, standard float (little endian)

#### Output Current 1 (outCurrent1)
- Description: output current from solar charger 1
- Size: 4 bytes
- Encoding: none, standard float (little endian)

#### Output Current 2 (outCurrent2)
- Description: output current form solar charger 2
- Size: 4 bytes
- Encoding: none, standard float (little endian)

#### Solar Packing
- Byte 0: Header (0xF0)
- Byte 1-4: outCurrent1
- Byte 5-8: outCurrent2
- Byte 9-12: totalCurrent
- Byte 13: 0x00 (unused)
- Byte 14: 0x00 (packet#)
- Byte 15: 8-bit checksum
