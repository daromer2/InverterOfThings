#ifndef INVERTER_H
#define INVERTER_H

#include <Arduino.h>

#define INVERTER_COMMAND_TIMEOUT_MS 5000
#define INVERTER_COMMAND_DELAY_MS 500

//Send and receive periodic inverter commands
void serviceInverter();

struct QpiMessage
{
  byte protocolId;
};

struct P003PSMessage
{
  unsigned long rxTimeSec;
  float solarWatt1;
  float solarWatt2;
  float batteryWatt;
  float acin2_r;
  float acin2_s;
  float acin2_t;
  float acin2_total;
  float w_r;
  float w_s;
  float w_t;
  float w_total;
  float va_r;
  float va_s;
  float va_t;
  float va_total;
  float ac_output_procent;  
};

struct P006FPADJMessage
{  //-- '34'^D0301,0000,1,0099,1,0109,1,0112⸮7'
  unsigned long rxTimeSec;
  float dir;
  float watt;
  float feedingGridDirectionR;
  float calibrationWattR;  
  float feedingGridDirectionS;
  float calibrationWattS; 
  float feedingGridDirectionT;
  float calibrationWattT; 
};

struct P003GSMessage
{
  unsigned long rxTimeSec;
  float solarInputV1;
  float solarInputV2;
  float solarInputA1;
  float solarInputA2;
  float battV;
  float battCapacity;
  float battA;
  float acInputVoltageR;
  float acInputVoltageS;
  float acInputVoltageT;
  float acInputFrequency;
  float acInputCurrentR;
  float acInputCurrentS;
  float acInputCurrentT;
  float acOutputVoltageR;
  float acOutputVoltageS;
  float acOutputVoltageT;
  float acOutputFrequency;
  float acOutputCurrentR;
  float acOutputCurrentS;
  float acOutputCurrentT;
};



struct QpigsMessage
{
  unsigned long rxTimeSec;
  //087.6 51.18 08.81 04.71 04.10 0450 +034 00.05 -030 0000 11000000O⸮'
  float solarV;
  float battV;
  float battChargeA;
  float solarA;
  float solar2A;
  float wattage;
}; 

//      GridV GridP  GridF GridI  ACoV. AcoP  AcoF AcoI  Loa% Bus1V Bus2V Batt1V Batt2v  BatCap Pv1P  Pv2P  Pv3P  Pv1V. Pv2V  Pv3V  Temp  Status
//QPIGS 228.8 001252 50.0  0000.0 000.0 00000 00.0 000.0 000 390.0  390.0 000.8  ---.-   000    01276 00000 ----- 329.7 006.7 ---.- 037.0 D---0000107?
//QPIGS 224.7 100860 50.0  0004.6 223.2 00837 50.0 003.7 018 429.9  429.9 057.0  ---.-   092    00241 00000 ----- 277.7 006.6 ---.- 035.0 A---1011019\xf8\r' 
//QPIGS 223.8 100874 50.0  0004.6 222.9 00848 50.0 003.8 018 431.1  431.1 057.0 ---.-    093    00263 00000 ----- 288.5 006.5 ---.- 035.0 A---101101\xd4z\r'
struct Qpigs55Message
{
  unsigned long rxTimeSec;
  //087.6 51.18 08.81 04.71 04.10 0450 +034 00.05 -030 0000 11000000O⸮'
  float gridAcV; //Grid AC Voltage
  float gridAcP; //Grid AC power
  float gridAcF; //Grid AC  Frequency
  float gridAcI; //Grid AC curret
  float l1AcV;   //L1 AC Output Voltage
  int   l1AcP;   //L1 AC Output power
  float l1AcF;   //L1 AC Output Frequency
  float l1AcI;   //L1 AC Output current
  int   l1AcLp;   //L1 AC Output load %
  float bus1V;  // BUS1 Voltage
  float bus2V;  // BUS2 Voltage
  float batt1V;  // Battery1 Voltage
  float batt2V;  // Battery2 Voltage ( String, ---.-)
  int   battCap; // Battery Capacity
  int   pv1P;      // PV 1 Input power
  int   pv2P;      // PV 2 Input power
  int   pv3P;      // PV 3 Input power ( String, ----)
  float pv1V;      // PV 1 Input voltage
  float pv2V;      // PV 2 Input voltage
  float pv3V;      // PV 3 Input voltage
}; 

struct QmodMessage
{
  char mode;
};

struct QpiwsMessage
{
  bool reserved0;
  bool inverterFault;
  bool busOver;
  bool busUnder;
  bool busSoftFail;
  bool lineFail;
  bool opvShort;
  bool overTemperature;
  bool fanLocked;
  bool batteryVoltageHigh;
  bool batteryLowAlarm;
  bool reserved13;
  bool batteryUnderShutdown;
  bool reserved15;
  bool overload;
  bool eepromFault;
  bool inverterOverCurrent;
  bool inverterSoftFail;
  bool selfTestFail;
  bool opDcVoltageOver;
  bool batOpen;
  bool currentSensorFail;
  bool batteryShort;
  bool powerLimit;
  bool pvVoltageHigh;
  bool mpptOverloadFault;
  bool mpptOverloadWarning;
  bool batteryTooLowToCharge;
  bool reserved30;
  bool reserved31;
};

struct QflagMessage
{
  bool disableBuzzer;
  bool enableOverloadBypass;
  bool enablePowerSaving;
  bool enableLcdEscape;
  bool enableOverloadRestart;
  bool enableOvertempRestart;
  bool enableBacklight;
  bool enablePrimarySourceInterruptedAlarm;
  bool enableFaultCodeRecording;
};

struct QidMessage
{
  char id[16];
};

#endif
