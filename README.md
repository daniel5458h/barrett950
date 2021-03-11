# Barrett 950 software for Teensy 3.2 and Nextion NX4832T035
 (c) Dr. Daniel Fernández, EA3HRU
 
 Notes:
  - Serial is the USB serial port for radio control from the PC
  - Serial1 is the serial port of the radio
  - Serial2 is the serial port for the Nextion touchscreen
    - Set NexHardware.cpp / nexInit() speed to 115200
    - Set Serial2 as serial port in NexConfig.h
