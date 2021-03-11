// Barrett 950 software for Teensy 3.2 and Nextion NX4832T035
// (c) Dr. Daniel Fernández, EA3HRU (daniel.fdez.mart@gmail.com). 
// 
// Notes:
//  - You need a 5V-tolerant Teensy, like the Teensy 3.2.
//  - Teensy pin 2 goes to the PTT Out / Receiver Cross Mute Out on the pin DB25 pin 21 of the radio
//  - Serial is the USB serial port for radio control from the PC
//  - Serial1 is the serial port of the radio
//    - You need a MAX232 level shifter or equivalent
//  - Serial2 is the serial port for the Nextion touchscreen
//    - Set NexHardware.cpp / nexInit() speed to 115200
//    - Set Serial2 as serial port in NexConfig.h
//
#include <Nextion.h>
#include <elapsedMillis.h>

#define XOFFTIMEOUT 1000    // Max duration of XON=0 (XOFF) until ignore it
#define LOGLINES 13         // Log lines
#define LOGLINESLENGTH 80   // Length of each line
#define DEBUGLEVEL 4        // A value of 0 prints only minimum info, 3 maximum info, 4 prints only UART messages, 5 prints UART along 3 (max info)
#define ppm_x10 26          // Measured frequency error in ppm*10
#define DIMHIGH 100         // High backligth level
#define DIMLOW 5            // Low backligth level
#define DIMTIMER 30000      // Timer to get the backligth low in auto mode
#define UARTRADIOSPEED 9900 // Set the radio usb speed (typ 9600)

bool XON=1;
bool freeFrequency=false;
int lastRealChannel=-1;
int currentChannel=-1;
char currentMode[4];
char lastRealMode[4];
char EN_HiPwr='H';
byte band=1;
bool enablePC=false;
elapsedMillis XOFFTimeStamp;
elapsedMillis autoDimTimer;
int pagenumber=0;
int pressNumber=0;
long RXfrequency, TXfrequency;

int digit10M_old, digit1M_old, digit100k_old, digit10k_old, digit1k_old, digit100_old;
bool digit10M_chg, digit1M_chg, digit100k_chg, digit10k_chg, digit1k_chg, digit100_chg;
int band_old;
char oldMode[4];
int NB;
int Tune;
int brigthMode=2; // 0-> min, 1-> max, 2-> auto

char logscreen[LOGLINES][LOGLINESLENGTH]={'\0'};

NexPage intro_page = NexPage(0, 0, "intro");
NexPage main_page = NexPage(1, 0, "main");
NexPage log_page = NexPage(2, 0, "log");

// Intro window elements
NexProgressBar j0a  = NexProgressBar(0, 1, "j0");

// Log window elements
NexText log_text0 = NexText(2, 2, "t0");
NexText log_text1 = NexText(2, 3, "t1");
NexText log_text2 = NexText(2, 4, "t2");
NexText log_text3 = NexText(2, 5, "t3");
NexText log_text4 = NexText(2, 6, "t4");
NexText log_text5 = NexText(2, 7, "t5");
NexText log_text6 = NexText(2, 8, "t6");
NexText log_text7 = NexText(2, 9, "t7");
NexText log_text8 = NexText(2, 10, "t8");
NexText log_text9 = NexText(2, 11, "t9");
NexText log_text10 = NexText(2, 12, "t10");
NexText log_text11 = NexText(2, 13, "t11");
NexText log_text12 = NexText(2, 14, "t12"); 
NexButton b0ret = NexButton(2,1, "b0");

// Main window elements
NexText tch = NexText(1, 9, "tch");

NexHotspot mch0up = NexHotspot(1,43, "mch0up");
NexHotspot mch0dn = NexHotspot(1,44, "mch0dn");
NexHotspot mch1up = NexHotspot(1,41, "mch1up");
NexHotspot mch1dn = NexHotspot(1,42, "mch1dn");
NexHotspot mch2up = NexHotspot(1,39, "mch2up");
NexHotspot mch2dn = NexHotspot(1,40, "mch2dn");

NexHotspot mfr0up = NexHotspot(1,26, "mfr0up");
NexHotspot mfr0dn = NexHotspot(1,27, "mfr0dn");
NexHotspot mfr1up = NexHotspot(1,28, "mfr1up");
NexHotspot mfr1dn = NexHotspot(1,29, "mfr1dn");
NexHotspot mfr2up = NexHotspot(1,32, "mfr2up");
NexHotspot mfr2dn = NexHotspot(1,33, "mfr2dn");
NexHotspot mfr3up = NexHotspot(1,31, "mfr3up");
NexHotspot mfr3dn = NexHotspot(1,30, "mfr3dn");
NexHotspot mfr4up = NexHotspot(1,35, "mfr4up");
NexHotspot mfr4dn = NexHotspot(1,34, "mfr4dn");
NexHotspot mfr5up = NexHotspot(1,36, "mfr5up");
NexHotspot mfr5dn = NexHotspot(1,37, "mfr5dn");
NexText dot0 = NexText(1, 8, "dot0");
NexText dot1 = NexText(1, 9, "dot1");

NexText t0 = NexText(1, 11, "t0");
NexText t1 = NexText(1, 12, "t1");
NexText t2 = NexText(1, 12, "t2");
NexText t3 = NexText(1, 15, "t3");
NexText t4 = NexText(1, 19, "t4");
NexText t5 = NexText(1, 20, "t5");
NexText t6 = NexText(1, 21, "t6");
NexText t7 = NexText(1, 23, "t7");
NexButton b0 = NexButton(1,10, "b0");
NexButton b1 = NexButton(1,14, "b1");
NexButton b2 = NexButton(1,16, "b2");
NexButton b3 = NexButton(1,17, "b3");
NexButton b4 = NexButton(1,18, "b4");
NexButton b5 = NexButton(1,22, "b5");
NexButton b6 = NexButton(1,24, "b6");
NexButton b7 = NexButton(1,25, "b7");
NexNumber frq5 = NexNumber(1,6, "frq5");
NexNumber frq4 = NexNumber(1,5, "frq4");
NexNumber frq3 = NexNumber(1,4, "frq3");
NexNumber frq2 = NexNumber(1,3, "frq2");
NexNumber frq1 = NexNumber(1,2, "frq1");
NexNumber frq0 = NexNumber(1,1, "frq0");

NexHotspot hot0    = NexHotspot(1, 46, "hot0");

NexTouch *nex_listen_list[] = 
{
    &mch2up,
    &mch2dn,
    &mch1up,
    &mch1dn,
    &mch0up,
    &mch0dn,
    &b0,
    &b1,
    &b2,
    &b3,
    &b4,
    &b5,
    &b6,
    &b7,
    &mfr0up,
    &mfr0dn,
    &mfr1up,
    &mfr1dn,
    &mfr2up,
    &mfr2dn,
    &mfr3up,
    &mfr3dn,
    &mfr4up,
    &mfr4dn,
    &mfr5up,
    &mfr5dn,
    &hot0,
    &b0ret,
    NULL
};

void writelog(char *message) {
  for(int i=1; i<LOGLINES; i++) strcpy(logscreen[i-1], logscreen[i]);
  strcpy(logscreen[LOGLINES-1], message);
}

void updatelog(void) {
  log_text0.setText(logscreen[0]);
  log_text1.setText(logscreen[1]);
  log_text2.setText(logscreen[2]);
  log_text3.setText(logscreen[3]);
  log_text4.setText(logscreen[4]);
  log_text5.setText(logscreen[5]);
  log_text6.setText(logscreen[6]);
  log_text7.setText(logscreen[7]);
  log_text8.setText(logscreen[8]);
  log_text9.setText(logscreen[9]);
  log_text10.setText(logscreen[10]);
  log_text11.setText(logscreen[11]);
  log_text12.setText(logscreen[12]);
}

void displaylog(void) {
  log_page.show();
  pagenumber=2;
  updatelog();
}

void clearScreenData(void) {
  digit10M_old=-1;
  digit1M_old=-1;
  digit100k_old=-1;
  digit10k_old=-1;
  digit1k_old=-1;
  digit100_old=-1;
  digit10M_chg=true;
  digit1M_chg=true;
  digit100k_chg=true;
  digit10k_chg=true;
  digit1k_chg=true;
  digit100_chg=true;
  band_old=-1;
  pressNumber=0;
  oldMode[0]='\0';
}

bool AllowRadioTransaction(void) {
  if(XON) return(true);
  else if(XOFFTimeStamp<XOFFTIMEOUT) {
    printerror(1, "ERROR: Communication timeout waiting for the radio XOFF.\n");
    return(false);   // A young XON=0 (XOFF) is correct
  } // An old one is not, we declare it as XON=1
    else {
      XON=true;
      return(true);
    }
}

bool readUSBSerialLine(char *bufptr)
{
  bool endTransaction=false;
  static int framebytes=0; // Total number of bytes read during the transaction
  char rcvData;
  char debugmessage[LOGLINESLENGTH];

  if(!Serial.available()) return(0);

  while (Serial.available())
  {
    endTransaction=false;
    rcvData=Serial.read();

    //Store in the string except if XON, XOFF, CR or LF
    if(rcvData != '\r' && rcvData != '\n' && rcvData!=0x13 && rcvData!=0x11) 
    {
      bufptr[framebytes]=rcvData;
      framebytes++;
    } 

    // End of transaction when CR is received
    if(rcvData=='\r') {
      bufptr[framebytes]='\0';
      endTransaction=true;
      framebytes=0;
      sprintf(debugmessage, "  RCV from  USB: %s", bufptr);
      printinfo(3, debugmessage);
    }
  }
  return(endTransaction);
}

void printerror(int severity, const char * format, ...)
{
  char error[80];
  va_list args;
  va_start(args, format);
  vsprintf(error, format, args);
  va_end(args); 
  writelog(error);
  if(pagenumber==2) updatelog();
  if(severity==-1)
  {
    displaylog();
    while(1);
  } 
}

void printinfo(int debug, const char * format, ...)
{
  if(debug>DEBUGLEVEL) return;
  if(DEBUGLEVEL==4 && debug!=3 ) return;   // Print only raw radio and USB messages

  char info[80];
  va_list args;
  va_start(args, format);
  vsprintf(info, format, args);
  va_end(args); 
  writelog(info);
  if(pagenumber==2) updatelog();
}

int readRadioSerialLine(char *bufptr, int wait)
{
  bool endTransaction=false;
  static int framebytes=0; // Total number of bytes read during the transaction
  char rcvData;
  char debugmessage[LOGLINESLENGTH];

  elapsedMillis timestamp_1;

  // No waiting and no data, return
  if(!wait && !Serial1.available()) return(0);

  while(!endTransaction)
  {
    // No waiting and no data, return
    if(!wait && !Serial1.available()) return(0);

    // If I have to wait, do it until timeout.
    if(wait) {
      timestamp_1=0;
      while(!Serial1.available() && timestamp_1<XOFFTIMEOUT);

      if(timestamp_1>=XOFFTIMEOUT) {
        printerror(1, "ERROR: Communication timeout waiting for the radio.\n");
        return(-1);
      }
    }

    endTransaction=false;
    rcvData=Serial1.read();

    // Flow control
    if(rcvData==0x11) {
      XON=true;   // We received XON, we can send data to radio
      //if(enablePC) Serial.write(0x11);         // We inform the USB port we can take more data
    }
    if(rcvData==0x13) {           // We received XOFF, we can't
      XON=false;
      XOFFTimeStamp=0;
      if(enablePC) Serial.write(0x13);         // We inform the USB port we can't take any more data
    }

    //Store in the string except if XON, XOFF, CR or LF
    if(rcvData != '\r' && rcvData != '\n' && rcvData!=0x13 && rcvData!=0x11) 
    {
      bufptr[framebytes]=rcvData;
      framebytes++;
    } 

    // End of transaction when XON is received
    if(rcvData==0x11) {
      bufptr[framebytes]='\0';
      endTransaction=true;
      framebytes=0;
      sprintf(debugmessage, "  RCV frm radio: %s", bufptr);
      printinfo(3, debugmessage);
    }
  }
  if(endTransaction) return(strlen(bufptr)); else return(0);
}

int writeUSBSerialLine(char *bufptr)
{
  int index=0;
  char debugmessage[LOGLINESLENGTH];

  while(bufptr[index]!='\0') Serial.write(bufptr[index++]);
  Serial.write('\r'); index++; // We send CR
  Serial.write('\n'); index++; // We send LF
  Serial.write(0x11); index++; // We send XON

  sprintf(debugmessage, "  SENT  to  USB: %s", bufptr);
  printinfo(3, debugmessage);

  return(index);
}

int writeRadioSerialLine(char *bufptr)
{
  int index=0;
  char debugmessage[LOGLINESLENGTH];

  while(bufptr[index]!='\0') Serial1.write(bufptr[index++]);   
  Serial1.write('\r'); index++; // We send CR

  sprintf(debugmessage, "  SENT to radio: %s", bufptr);
  printinfo(3, debugmessage);

  return(index);
}

bool getCommandStatus(void)
{
  char buffer_1[20];

  int i=readRadioSerialLine(buffer_1,1);

  if(!strcmp(buffer_1, "OK")) return(false);
  else 
  {
    if(i>0) printerror(1, "ERROR: Received message %s, legnth %d.\n", buffer_1,i);
//    Serial.println(buffer_1);
    return(true);
  }
}

long getRXFrequency(void)
{
  char buff[20];
  long long CorrectedFrequency;
  
  printinfo(2, "Reading RX frequency:\n");
  sprintf(buff, "IR");
  writeRadioSerialLine(buff);
  int read = readRadioSerialLine(buff,1);
  if(read!=8)
  {
    printerror(1, "ERROR: Not enough data received when reading the RX frequency.\n");
    return(-1);
  }

  CorrectedFrequency=atol(buff);
  printinfo(2, " Raw current RX frequency: %li\n", CorrectedFrequency);
  CorrectedFrequency=10000000*CorrectedFrequency/(10000000+ppm_x10);
  if((CorrectedFrequency%100)>50) CorrectedFrequency=CorrectedFrequency/100*100+100; else CorrectedFrequency=CorrectedFrequency/100*100;
  printinfo(2, " Corrected current RX frequency: %li\n", CorrectedFrequency);

  RXfrequency=CorrectedFrequency;
  return(CorrectedFrequency);
}

long getTXFrequency(void)
{
  char buff[20];
  long long CorrectedFrequency;

  printinfo(2, "Reading TX frequency:\n");
  sprintf(buff, "IT");
  writeRadioSerialLine(buff);
  int read = readRadioSerialLine(buff,1);
  if(read!=8)
  {
    printerror(1, "ERROR: Not enough data received when reading the TX frequency.\n");
    return(-1);
  }

  CorrectedFrequency=atol(buff);
  printinfo(2, " Raw current TX frequency: %li\n", CorrectedFrequency);
  CorrectedFrequency=10000000*CorrectedFrequency/(10000000+ppm_x10);
  if((CorrectedFrequency%100)>50) CorrectedFrequency=CorrectedFrequency/100*100+100; else CorrectedFrequency=CorrectedFrequency/100*100;
  printinfo(2, " Corrected current TX frequency: %li\n", CorrectedFrequency);

  TXfrequency=CorrectedFrequency;
  return(CorrectedFrequency);
}

bool getMode(char *bufptr)
{
  char buff[20];

  printinfo(2, "Reading mode:\n");

  sprintf(buff, "IB");
  writeRadioSerialLine(buff);
  int read = readRadioSerialLine(buff, 1);
  if(read!=2)
  {
    printerror(1, "ERROR: Not enough data received when reading the mode.\n");
    return(true);
  }
  if(!strcmp(buff, "BL")) strcpy(bufptr, "LSB");
  else if(!strcmp(buff, "BU")) strcpy(bufptr, "USB");
  else if(!strcmp(buff, "BA")) strcpy(bufptr, "AM");
  else if(!strcmp(buff, "BC")) strcpy(bufptr, "CW");
  else if(!strcmp(buff, "BF")) strcpy(bufptr, "FSK");
  else 
  {
    printerror(1, "ERROR: Unknown mode setting received from the radio.\n");
    bufptr[0]='\0';
    return(true);
  }

  printinfo(2, " Current mode: %s\n", bufptr);
  strcpy(currentMode,bufptr);
  return(false);
}

bool setMode(char *bufptr)
{
  char Mode;
  char str[4];

  if(!strcmp(bufptr, "LSB")) Mode='L';
  else if(!strcmp(bufptr, "USB")) Mode='U';
  else if(!strcmp(bufptr, "AM")) Mode='A';
  //else if(!strcmp(bufptr, "CW")) Mode='C';    // CW mode not available
  //else if(!strcmp(bufptr, "FSK")) Mode='F';   // FSK mode not available
  else 
  {
    printerror(1, "ERROR: Attempt to select a invalid mode: %s.\n", bufptr);
    return(true); 
  }
  
  sprintf(str, "XB%c",Mode);
  printinfo(1, "Set mode: %s.\n",bufptr);
  writeRadioSerialLine(str); 

  if(!getCommandStatus()) { // Mode was correctly set
    strcpy(currentMode,bufptr);
    return(false);
  } else return(true);
}

int getChannel(void)
{
  char buff[20];
  
  printinfo(2, "Reading channel:\n");
  sprintf(buff, "IC");
  writeRadioSerialLine(buff);
  int read = readRadioSerialLine(buff,1);
  if(read!=4)
  {
    printerror(1, "ERROR: Not enough data received when reading the channel number.\n");
    return(-1);
  }

  currentChannel=atoi(buff);
  printinfo(2, " Current channel: %d\n", currentChannel);
  
  return(currentChannel);
}

bool setChannel(int channel)
{
  char str[20];

  if(channel<=0 || channel>450) 
  {
    printerror(1, "ERROR: Attempt to select a channel number ousite the programming range [1~450].\n");
    return(true);
  }
  sprintf(str, "XC%04d",channel);
  printinfo(1, "Set channel: %d\n",channel);
  writeRadioSerialLine(str);
  return(getCommandStatus());
}

bool setMute(char mute_mode)
{
  char str[20];

  if(mute_mode!='N' && mute_mode!='A' && mute_mode!='S' && mute_mode!='O') 
  {
    printerror(1, "ERROR: Attempt to select an ivalid mute mode [O,N,A,S].\n");
    return(true);
  }
  // Bug patching
  sprintf(str, "XMO");
  writeRadioSerialLine(str);
  getCommandStatus();

  sprintf(str, "XM%c",mute_mode);
  printinfo(1, "Set mute mode: %c.\n",mute_mode);
  writeRadioSerialLine(str);
  return(getCommandStatus());
}

char getMute(void)
{
  char buff[20];
  char Mute;

  printinfo(1, "Reading mute mode:\n");
  sprintf(buff, "IM");
  writeRadioSerialLine(buff);
  int read = readRadioSerialLine(buff, 1);
  if(read!=1)
  {
    printerror(1, "ERROR: Not enough data received when reading the mute mode.\n");
    return(-1);
  }

  // Patch for a reading bug
  if(buff[0]=='O') Mute='O';
  if(buff[0]=='S') Mute='S';
  if(buff[0]=='U') Mute='A';
  if(buff[0]=='A') Mute='N';

  printinfo(1, " Current mute mode: %c\n", Mute);

  return(Mute);
}

bool setPTT(int PTT_ON)
{
  char buff[20];
  
  if(PTT_ON==1) 
  {
    printinfo(1, "PTT is ON.\n");
    sprintf(buff, "XP1");
    writeRadioSerialLine(buff);
    return(getCommandStatus());
  }
  else
  {
    printinfo(1, "PTT is OFF.\n");
    sprintf(buff, "XP0");
    writeRadioSerialLine(buff);
    return(getCommandStatus());
  }
}

bool setNB(int NB_ON)
{
  char buff[20];
  
  if(NB_ON==1) 
  {
    printinfo(1, "NB is ON.\n");
    sprintf(buff, "ENY");
    writeRadioSerialLine(buff);
    if(getCommandStatus()==0) {
      NB=1;
      return(false);
    } else return true;
  }
  else
  {
    printinfo(1, "NB is OFF.\n");
    sprintf(buff, "ENN");
    writeRadioSerialLine(buff);
    if(getCommandStatus()==0) {
      NB=0;
      return(false);
    } else return true;
  }
}

bool setTune(int Tune_ON)
{
  char buff[20];
  
  if(Tune_ON==1) 
  {
    printinfo(1, "Tune is ON.\n");
    sprintf(buff, "EWY");
    writeRadioSerialLine(buff);
    if(getCommandStatus()==0) {
      Tune=1;
      return(false);
    } else return true;
  }
  else
  {
    printinfo(1, "Tune is OFF.\n");
    sprintf(buff, "EWN");
    writeRadioSerialLine(buff);
    if(getCommandStatus()==0) {
      Tune=0;
      return(false);
    } else return true;
  }
}

void printFrequency(long frequency, bool RX)
{
  int digit10M, digit1M, digit100k, digit10k, digit1k, digit100;
  uint32_t color;
  static bool RX_old=true;

  if(RX) color=65535; else color=63488;
  if(RX!=RX_old) clearScreenData();
  RX_old=RX;
  
  digit100=(frequency/100)%10;
  if(digit100_old!=digit100) {
    frq5.setValue(digit100);
    digit100_old=digit100;
    digit100_chg=true; 
  }
  digit1k=(frequency/1000)%10;
  if(digit1k_old!=digit1k) {
    frq4.setValue(digit1k);
    digit1k_old=digit1k;
    digit1k_chg=true;
  }
  digit10k=(frequency/10000)%10;
  if(digit10k_old!=digit10k) {
    frq3.setValue(digit10k);
    digit10k_old=digit10k;
    digit10k_chg=true;
  }
  digit100k=(frequency/100000)%10;
  if(digit100k_old!=digit100k) {
    frq2.setValue(digit100k);
    digit100k_old=digit100k;
    digit100k_chg=true;
  }
  digit1M=(frequency/1000000)%10;
  if(digit1M_old!=digit1M) {
    frq1.setValue(digit1M);
    digit1M_old=digit1M;
    digit1M_chg=true;
  }
  digit10M=(frequency/10000000)%10;
  if(digit10M_old!=digit10M) {
    frq0.setValue(digit10M);
    digit10M_old=digit10M;
    digit10M_chg=true;
  }

  if(digit10M==0 && digit10M_chg) {
    frq0.Set_font_color_pco((uint32_t) 23243);
    if(digit1M==0 && digit1M_chg) {
      frq1.Set_font_color_pco((uint32_t) 23243);
      dot0.Set_font_color_pco((uint32_t) 23243);
      if(digit100k==0 && digit100k_chg) {
        frq2.Set_font_color_pco((uint32_t) 23243);
        frq3.Set_font_color_pco((uint32_t) 23243);
        frq4.Set_font_color_pco((uint32_t) 23243);
        frq5.Set_font_color_pco((uint32_t) 23243);
        dot1.Set_font_color_pco((uint32_t) 23243);
      } else {
        if(digit100k_chg) frq2.Set_font_color_pco((uint32_t) color);
        if(digit10k_chg)  frq3.Set_font_color_pco((uint32_t) color);
        if(digit1k_chg)   frq4.Set_font_color_pco((uint32_t) color);
        if(digit100_chg)  frq5.Set_font_color_pco((uint32_t) color);
        if(digit100k_chg) dot1.Set_font_color_pco((uint32_t) color);
      } }
    else {
      if(digit1M_chg)   frq1.Set_font_color_pco((uint32_t) color);
      if(digit1M_chg)   dot0.Set_font_color_pco((uint32_t) color);
      if(digit100k_chg) frq2.Set_font_color_pco((uint32_t) color);
      if(digit10k_chg)  frq3.Set_font_color_pco((uint32_t) color);
      if(digit1k_chg)   frq4.Set_font_color_pco((uint32_t) color);
      if(digit100_chg)  frq5.Set_font_color_pco((uint32_t) color);
      if(digit100k_chg) dot1.Set_font_color_pco((uint32_t) color);
  }} else {
    if(digit10M_chg)  frq0.Set_font_color_pco((uint32_t) color);
    if(digit1M_chg)   frq1.Set_font_color_pco((uint32_t) color);
    if(digit1M_chg)   dot0.Set_font_color_pco((uint32_t) color);
    if(digit100k_chg) frq2.Set_font_color_pco((uint32_t) color);
    if(digit10k_chg)  frq3.Set_font_color_pco((uint32_t) color);
    if(digit1k_chg)   frq4.Set_font_color_pco((uint32_t) color);
    if(digit100_chg)  frq5.Set_font_color_pco((uint32_t) color);
    if(digit100k_chg) dot1.Set_font_color_pco((uint32_t) color);
  }
}

void screenTouched(void) {
  if(brigthMode==2) {
    setBrigthness(DIMHIGH);
    autoDimTimer=0;
  }
}

void printMode(char *mode)
{
  if(strcmp(oldMode,mode)) {
    t0.setText(mode);
    strcpy(oldMode,mode);
  }
}

void modeTouch(void) {
  char mode[4];

  getMode(mode);

  if(!strcmp(mode,"USB")) strcpy(mode,"LSB");
  else if(!strcmp(mode,"LSB")) strcpy(mode,"AM");
  else if(!strcmp(mode,"AM")) strcpy(mode,"USB");

  // Update screen if ok
  if(setMode(mode)==0) printMode(mode);
}

void b0PopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Key MODE pressed.\n");
  modeTouch();
}

void printMute(char mute)
{
  if(mute=='O') {
    t2.Set_font_color_pco((uint32_t) 23243);
    t2.setText("OFF");
  }
  if(mute=='A') {
    t2.Set_font_color_pco((uint32_t) 65535);
    t2.setText("RF");
  }
  if(mute=='N') {
    t2.Set_font_color_pco((uint32_t) 65535);
    t2.setText("VOICE");
  }
}

void muteTouch(void) {
  char mute;

  mute=getMute();

  if(mute=='O') mute='A'; 
  else if(mute=='A') mute='N'; 
  else if(mute=='N') mute='O'; 

  // Update screen if ok
  if(setMute(mute)==0) printMute(mute);
}

void b2PopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Key SQL pressed.\n");
  muteTouch();
}

void printNB(void)
{
  if(NB) {
    t1.setText("ON");
    t1.Set_font_color_pco((uint32_t) 65535);
  } else {
    t1.setText("OFF"); 
    t1.Set_font_color_pco((uint32_t) 23243);
  }
}

void NBTouch(void) {
  if(NB) setNB(0); else setNB(1);
  printNB();
}

void printTune(void)
{
  if(Tune) {
    t6.setText("ON");
    t6.Set_font_color_pco((uint32_t) 65535);
  } else {
    t6.setText("OFF"); 
    t6.Set_font_color_pco((uint32_t) 23243);
  }
}

void tuneTouch(void) {
  if(Tune) setTune(0); else setTune(1);
  printTune();
}

void b1PopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Key NB pressed.\n");
  NBTouch();
}

void printChannel(int channel)
{
  char str[4];

  if(channel<1) return;

  // Set grey color if it is inside the free frequency slot
  if(channel>=240 && channel<440) tch.Set_font_color_pco((uint32_t) 23243); else tch.Set_font_color_pco((uint32_t) 65535);

  sprintf(str, "%03d",channel);
  tch.setText(str);
  RXfrequency=getRXFrequency();
  TXfrequency=getTXFrequency();
  printFrequency(RXfrequency,true);
  getMode(str);
  printMode(str);
  updateBand(RXfrequency);
}

void channelTouch(int amount) {
  int channel;
  
  // We recover the previous channel if we were in  free frequency mode
  if(freeFrequency){
    freeFrequency=false;
    setChannel(lastRealChannel);
    setMode(lastRealMode);
    printChannel(lastRealChannel);
    return;
  }

  channel=getChannel();
  channel+=amount;

  // Round it
  if(channel>450) channel-=450;
  if(channel<1) channel+=450;
  if(channel==0) channel=1;

  // Update screen if ok
  if(setChannel(channel)==0) {
    printChannel(channel);
  }
}

void mch2upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot CH+1 pressed.\n");
  channelTouch(1);
}

void mch2dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot CH-1 pressed.\n");
  channelTouch(-1);
}

void mch1upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot CH+10 pressed.\n");
  channelTouch(10);
}

void mch1dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot CH-10 pressed.\n");
  channelTouch(-10);
}

void mch0upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot CH+100 pressed.\n");
  channelTouch(100);
}

void mch0dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot CH-100 pressed.\n");
  channelTouch(-100);
}

void frequencyTouch(long amount) {
  char str[40];
  static long frequency;
  long CorrectedFrequency;
  static int channel;
  static int channel_writecounter;

  // We store the previous channel before changing the frequency
  if(!freeFrequency){
    lastRealChannel=getChannel();
    getMode(lastRealMode);
    channel=240+millis()%200;
    channel_writecounter=0;   // Number of times we've written in any specific channel
    frequency=getRXFrequency();
    if(frequency==-1) return;
    freeFrequency=true;
  }
  
//  frequency=getRXFrequency();
  frequency+=amount;

  // Limit ranges
  if(frequency>29999000)frequency=29999000;
  if(frequency<501000) frequency=501000;

  CorrectedFrequency=frequency+(frequency/100000*ppm_x10)/100;
  if((CorrectedFrequency%10)>5) CorrectedFrequency=CorrectedFrequency/10*10+10; else CorrectedFrequency=CorrectedFrequency/10*10;

  //getMode(mode);

  sprintf(str,"PC%04dR%08liT%08liZNSNH%cAN", channel, CorrectedFrequency, CorrectedFrequency, EN_HiPwr);
  writeRadioSerialLine(str);

  // Update screen if ok
  if(getCommandStatus()==0) {
    setMode(currentMode);
    // If it is a new channel, we update all info in the screen
    if(!channel_writecounter) printChannel(channel);
    else {  // Otherwise, just frequency and band
      RXfrequency=frequency; TXfrequency=frequency;
      printFrequency(RXfrequency,true);
      updateBand(RXfrequency);
    }
    // We reuse the channel number 100 times before we change it.
    if(channel_writecounter++>100) {
      channel++;
      channel_writecounter=0;
    }
    if(channel>440) channel=240;
  }
}

void mfr0upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR+10M pressed.\n");
  frequencyTouch(10000000);
}

void mfr0dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR-10M pressed.\n");
  frequencyTouch(-10000000);
}

void mfr1upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR+1M pressed.\n");
  frequencyTouch(1000000);
}

void mfr1dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR-1M pressed.\n");
  frequencyTouch(-1000000);
}

void mfr2upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR+100k pressed.\n");
  frequencyTouch(100000);
}

void mfr2dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR-100k pressed.\n");
  frequencyTouch(-100000);
}

void mfr3upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR+10k pressed.\n");
  frequencyTouch(10000);
}

void mfr3dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR-10k pressed.\n");
  frequencyTouch(-10000);
}

void mfr4upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR+1k pressed.\n");
  frequencyTouch(1000);
}

void mfr4dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR-1k pressed.\n");
  frequencyTouch(-1000);
}

void mfr5upPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR+100 pressed.\n");
  frequencyTouch(100);
}

void mfr5dnPopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Hotspot FR-100 pressed.\n");
  frequencyTouch(-100);
}

void printPWR(void) {
  if(EN_HiPwr=='H') t3.setText("HIGH"); else t3.setText("LOW");
}

void PWRTouch(void) {
  char str[10];
  char mode[4];
  int channel=getChannel();

  getMode(mode);

  if(EN_HiPwr=='H') {
    sprintf(str,"PC%04dHL", channel);
    writeRadioSerialLine(str);
    if(getCommandStatus()==0) {
      EN_HiPwr='L';
      printPWR();
      setMode(mode);
    }
  }
  else {
    sprintf(str,"PC%04dHH", channel);
    writeRadioSerialLine(str);
    if(getCommandStatus()==0) {
      EN_HiPwr='H';
      printPWR();
      setMode(mode);
    }
  }
}

void b3PopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Key PWR pressed.\n");
  PWRTouch();
}

void printBand(int band) {
  if(band==band_old) return;
  band_old=band;
  
  if(!band) {
    t5.setText("NONE");
    t5.Set_font_color_pco((uint32_t) 23243);
  } else {
    t5.Set_font_color_pco((uint32_t) 65535);
    if(band==1) {
      t5.setText("160 m");
    }
    if(band==2) {
      t5.setText("80 m");
    }
    if(band==3) {
      t5.setText("40 m");
    }
    if(band==4) {
      t5.setText("30 m");
    }
    if(band==5) {
      t5.setText("20 m");
    }
    if(band==6) {
      t5.setText("17 m");
    }
    if(band==7) {
      t5.setText("15 m");
    }
    if(band==8) {
      t5.setText("12 m");
    }
    if(band==9) {
      t5.setText("11 m");
    }
    if(band==10) {
      t5.setText("10 m");
    }
  }
}

void updateBand(long frequency) {
  int band=0;
  
  if(frequency>=1800000 && frequency<=2000000) band=1;
  if(frequency>=3500000 && frequency<=3800000) band=2;
  if(frequency>=7000000 && frequency<=7200000) band=3;
  if(frequency>=10100000 && frequency<=10150000) band=4;
  if(frequency>=14000000 && frequency<=14350000) band=5;
  if(frequency>=18068000 && frequency<=18168000) band=6;
  if(frequency>=21000000 && frequency<=21450000) band=7;
  if(frequency>=24890000 && frequency<=24990000) band=8;
  if(frequency>=26950000 && frequency<=27405000) band=9;
  if(frequency>=28000000 && frequency<=29999000) band=10;

  printBand(band);
}

void BandTouch(void) {
  if(getChannel()>=441) if(++band>10) band=1;

  freeFrequency=false;
  setChannel(440+band);
  printChannel(440+band);
}

void b5PopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Key BAND pressed.\n");
  BandTouch();
}

void ProgTouch(void) {
  static long RXfrequency;
  static long TXfrequency;
  static char mode[4];
  int channel;
  long CorrectedRXFrequency;
  long CorrectedTXFrequency;
  char str[40];

  if(pressNumber==0) {
    t4.setText("RXF?");
    t4.Set_font_color_pco((uint32_t) 65535);
    b4.setText("SET");
    printinfo(1,"Key PROG pressed, asking for RX frequency.\n");
  }
  if(pressNumber==1) {
    RXfrequency=getRXFrequency();
    getMode(mode);
    t4.setText("TXF?");
    printinfo(1,"Key PROG pressed, RX frequency selected, asking for TXF.\n");
  }
  if(pressNumber==2) {
    TXfrequency=getRXFrequency();
    getMode(mode);
    t4.setText("CHN?");
    printinfo(1,"Key PROG pressed, TX frequency selected, asking for CHN.\n");
  }
  if(pressNumber==3) {
    t4.setText("DONE"); 
    b4.setText("PROG");
    channel=getChannel();
    
    CorrectedRXFrequency=RXfrequency+(RXfrequency/100000*ppm_x10)/100;
    if((CorrectedRXFrequency%10)>5) CorrectedRXFrequency=CorrectedRXFrequency/10*10+10; else CorrectedRXFrequency=CorrectedRXFrequency/10*10;
    CorrectedTXFrequency=TXfrequency+(TXfrequency/100000*ppm_x10)/100;
    if((CorrectedTXFrequency%10)>5) CorrectedTXFrequency=CorrectedTXFrequency/10*10+10; else CorrectedTXFrequency=CorrectedTXFrequency/10*10;

    sprintf(str,"PC%04dR%08liT%08liZNSNH%cANB%c", channel, CorrectedRXFrequency, CorrectedTXFrequency, EN_HiPwr, mode[0]);
    writeRadioSerialLine(str); //getCommandStatus();

    // Update screen if ok
    if(getCommandStatus()==0) {
      setMode(mode);
      printChannel(channel);
      t4.Set_font_color_pco((uint32_t) 23243);
      t4.setText("OFF");
      printinfo(1,"Key PROG pressed, channel selected, memory stored.\n");
    } else {
      t4.setText("ERROR");
      printinfo(1,"Key PROG pressed, ERROR storing memory.\n");
    }
  }
  if(++pressNumber>3) pressNumber=0; 
}

void setBrigthness(int brigthness) {
  char buff[20];

  sprintf(buff, "dim=%d\xFF\xFF\xFF", brigthness);
  Serial2.print(buff);
}

void printBrigthMode(void) {
  if(brigthMode==0) t7.setText("MIN");
  if(brigthMode==1) t7.setText("MAX");
  if(brigthMode==2) t7.setText("AUTO");
}

void BrigthTouch(void) {
  if(++brigthMode>2) brigthMode=0;

  if(brigthMode==0) {
    setBrigthness(DIMLOW);
    printBrigthMode();
  }
  if(brigthMode==1) {
    setBrigthness(DIMHIGH);
    printBrigthMode();
  }
  if(brigthMode==2) {
    setBrigthness(DIMHIGH);
    printBrigthMode();
    autoDimTimer=0;
  }
}

void b4PopCallback(void *ptr) {
  screenTouched();
  ProgTouch();
}

void b7PopCallback(void *ptr) {
  screenTouched();
  BrigthTouch();
}

void b6PopCallback(void *ptr) {
  screenTouched();
  printinfo(1,"Key TUNE pressed.\n");
  tuneTouch();
}

void hot0PopCallback(void *ptr) {
  screenTouched();
  displaylog();
}

void b0retPopCallback(void *ptr) {
  screenTouched();
  main_page.show();
  pagenumber=1;
  clearScreenData();
  printNB();
  printMute(getMute());
  printPWR();
  printTune();
  printBrigthMode();
  printChannel(getChannel());
}

void setup()
{
  uint32_t progress;
  char buff[20];
  
  Serial.begin(9600);             // USB 
  Serial1.begin(UARTRADIOSPEED);  // Radio

  pinMode(2, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  
  nexInit();
  setBrigthness(DIMHIGH);

  delay(100);
  for(progress=0; progress<100; progress++) {
    j0a.setValue(progress);
    delay(100);
    if(progress==90) {
      while(Serial1.available()) Serial1.read(); // Clear buffer
      buff[0]='\0';
      writeRadioSerialLine(buff);
      if(getCommandStatus()) {
        printerror(-1, "ERROR: Radio is not responding.\n");
        while(1);
      }
    }
  }
  printinfo(1,"Radio is detected correctly.\n");

  main_page.show();
  pagenumber=1;
  clearScreenData();

  // Set radio defaults
  if(setNB(0)==0) printNB();
  if(setTune(0)==0) printTune();
  if(setMute('O')==0) printMute('O');
  screenTouched();
  printBrigthMode();
  printChannel(getChannel());
  
  mch2up.attachPop(mch2upPopCallback, &mch2up);
  mch2dn.attachPop(mch2dnPopCallback, &mch2dn);
  mch1up.attachPop(mch1upPopCallback, &mch1up);
  mch1dn.attachPop(mch1dnPopCallback, &mch1dn);
  mch0up.attachPop(mch0upPopCallback, &mch0up);
  mch0dn.attachPop(mch0dnPopCallback, &mch0dn);
  
  b0.attachPop(b0PopCallback, &b0);
  b1.attachPop(b1PopCallback, &b1);
  b2.attachPop(b2PopCallback, &b2);
  b3.attachPop(b3PopCallback, &b3);
  b4.attachPop(b4PopCallback, &b4);
  b5.attachPop(b5PopCallback, &b5);
  b6.attachPop(b6PopCallback, &b6);
  b7.attachPop(b7PopCallback, &b7);

  mfr0up.attachPop(mfr0upPopCallback, &mfr0up);
  mfr0dn.attachPop(mfr0dnPopCallback, &mfr0dn);
  mfr1up.attachPop(mfr1upPopCallback, &mfr1up);
  mfr1dn.attachPop(mfr1dnPopCallback, &mfr1dn);
  mfr2up.attachPop(mfr2upPopCallback, &mfr2up);
  mfr2dn.attachPop(mfr2dnPopCallback, &mfr2dn);
  mfr3up.attachPop(mfr3upPopCallback, &mfr3up);
  mfr3dn.attachPop(mfr3dnPopCallback, &mfr3dn);
  mfr4up.attachPop(mfr4upPopCallback, &mfr4up);
  mfr4dn.attachPop(mfr4dnPopCallback, &mfr4dn);
  mfr5up.attachPop(mfr5upPopCallback, &mfr5up);
  mfr5dn.attachPop(mfr5dnPopCallback, &mfr5dn);

  hot0.attachPop(hot0PopCallback, &hot0);
  b0ret.attachPop(b0retPopCallback, &b0ret);
}

void loop()
{
  static char bufferToRadio[200];
  static char bufferToUSB[200];
  static elapsedMillis timestamp_1;
  static bool PTT_old=false;
  static bool enablePC_old=false;
  bool PTT=false;

  // Receive data from USB and transmit to radio
  if(AllowRadioTransaction()) {
    if(readUSBSerialLine(bufferToRadio)) {
      enablePC=true;
      timestamp_1=0;
      writeRadioSerialLine(bufferToRadio);
      printinfo(1,"Remote command: %s", bufferToRadio);
    }
  }

  // Receive data from radio and send to USB
  if(enablePC) {
    if(readRadioSerialLine(bufferToUSB,0)) {
      writeUSBSerialLine(bufferToUSB);
      printinfo(1,"Radio answers : %s", bufferToUSB);
    }
  }
  
  // If no data from USB, use the screen, otherwise, lock receiving data
  if(!enablePC) nexLoop(nex_listen_list);

  // If received data on the USB for a while, disable screen locking
  if(timestamp_1>XOFFTIMEOUT) enablePC=false;

  // We update the screen if there has been USB activity
  if(!enablePC && enablePC_old && pagenumber==1) {
    clearScreenData();
    printNB();
    printMute(getMute());
    printPWR();
    printTune();
    printBrigthMode();
    printChannel(getChannel());
  }

  if(autoDimTimer>DIMTIMER && brigthMode==2) setBrigthness(DIMLOW);

  // PTT routine
  if(digitalRead(2)) PTT=false; else PTT=true;
  if(PTT && !PTT_old) { // PTT pressed
    digitalWrite(13,true);
    printFrequency(TXfrequency,false);
    screenTouched();
    printinfo(1,"PTT is ON");
  }
  if(!PTT && PTT_old) { // PTT released
    printFrequency(RXfrequency,true);
    digitalWrite(13,false);
    screenTouched();
    printinfo(1,"PTT is OFF");
  }
  PTT_old=PTT;
  enablePC_old=enablePC;
}
