# Barrett 950 software for Teensy 3.2 and Nextion NX4832T035 touchscreen
(c) Dr. Daniel Fernández, EA3HRU
 
Notes:
  * Serial is the USB serial port for radio control from the PC
  * Serial1 is the serial port of the radio. You need a MAX232 level shifter or equivalent.
  * Serial2 is the serial port for the Nextion touchscreen
    * Set NexHardware.cpp / nexInit() speed to 115200
    * Set Serial2 as serial port in NexConfig.h 
 ---
 User manual:
 -----------
 **Interface**
 
 There are three pages on the nextion screen:
  * Intro page: Check thats the radio is there during power-up. Then it goes to the Main page.
  * Main page: Allows control of the radio.
  * Debug page: Allows debugging the system and snigg RS232 and USB transactions. Debugging level is set in the Teensy code. For accessing the debug page, press the upper-left corner of the main screen. To return to the main page, press the _Return to main_ button.

**Main page**

* You can click on the channel digits to change the channel. If you click in the upper part of the digit, its value will increase. If you click in the bottom part, its value will decrease. 
* You can also tune frequencies directly by touching the frequency digits. As the radio is channelized, this will cause programming of a random channel number in the range 240~439 with the new frequency set. In order to not overwork the EEPROM, the channel will automatically increase when it has been written 100 times.
* MODE sets the radio mode among AM, USB and LSB.
* NB enables and disables the noise blanker function.
* SQL enables the sradio squelch among OFF, RF and VOICE.
* PWR sets the radio power HIGH or LOW.
* PROG allows channel programming and split operation. When pressed for the first time, its value changes from OFF to RXF? and the the RX frequency should be set by the user. When pressed again, it asks for the TXF? and the TX frequency should be set. When pressed for a thrid time, it displays CHN? and the channel number should be set.
* BAND allows for setting the default channels 441~450, currently programmed with each ham radio band lowest frequency.
* TUNE enables the external tuner.
* DIM allows reducing the screen brigthness. It cycles from MAX, to AUTO and to LOW. AUTO will use MAX brigthness until some time has passed, and then it goes to LOW. If there is screen activity, it will return to MAX.

**Other functions**
* You can control the radio via USB using the standard radio commands.
* You can correct the frequency offset of the radio by using the _ppm_x10_ constant defined in the Teensy code. 
* You can correct the RS232 timing errors of the radio by using the _UARTRADIOSPEED_ constant defined in the Teensy code. 
* You can set the information level printed to the debug page by setting the _DEBUGLEVEL_ constant defined in the Teensy code. 
