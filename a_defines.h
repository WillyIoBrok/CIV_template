#ifndef defines_h
#define defines_h

/* 
CIV_template - a_Defines.h / general definitions
*/

#define VERSION_STRING "CIV_template V0_3 May 28th, 2022"

// if defined, debug messages on the serial line will be generated
// default: active, i.e.uncommented
#define debug 

// if defined, also the Modulation Mode parameters of the radio are processed
// default: inactive, i.e. commented out (since this is not needed in most of the cases)
//#define modmode

// if defined, a triggerinput on (Pin 16 of ESP32) is used for exact time measurements
// this is useful for checking the time delay of the PTT signal generated by ESP32
// !!! only possible, when using BT on ESP32, otherwise these pins are used as COM2 for CIV
//#define ESPtriggerMeas

// Global compile switches ===================================================================================


// some general defines --------------------------------------------------------------------------------------

// HW pins in use:

// Arduino uno, nano, Pro mini etc.
// Pin 08 and 09    Altsoftserial COM for CIV HW onewire

// Arduino 2560
// Pin 18 and 19    COM1 for CIV HW onewire

// ESP32:
// Pin 16 and 17    COM2 for CIV HW onewire 
// Pin 16           Trigger for timing measurements (debugging) in case of CIV via Bluetooth

//------------------------------------------------------------------------------------------------------------
// times and timers ...

// repetition rate of the base loop, i.e. every 10ms an action can/will be taken
#define BASELOOP_TICK 10 

// this is the maximum time in ms which the radio may take for answering a query
#define t_waitForAnswer 900
// the same in "loop runs" ...
#define lp_waitForAnswer (t_waitForAnswer/BASELOOP_TICK)

// this is the polling time in ms for the RXTX query
// note: in fastPTT mode this may be significantly slower, since it doesn't influence the PTT delay in this case
#ifdef fastPTT
  #define t_RXTXquery 230
#else
  #define t_RXTXquery 80
#endif
// the same in "loop runs" ...
#define lp_RXTXquery (t_RXTXquery/BASELOOP_TICK)

// this is the polling time in ms for the slow poll queries of frequency, ModMode
#define t_slowQuery 1000
// the same in "loop runs" ...
#define lp_slowQuery (t_slowQuery/BASELOOP_TICK)

// this is the waiting time after sending a command before processing an answer 
// only necessary in case of fast polling the RXTX state
#ifndef fastPTT
  #define t_gapAfterquery 20
  // the same in "loop runs" ...
  #define lp_gapAfterquery (t_gapAfterquery/BASELOOP_TICK)
#endif

//------------------------------------------------------------------------------------------------------------

enum onOff_t:uint8_t {
	OFF = 0,
	ON  = 1,
  NDEF
};

enum radioOnOff_t:uint8_t {
  RADIO_OFF = 0,
  RADIO_ON  = 1,
  RADIO_OFF_TR,     // transit from OFF to ON
  RADIO_ON_TR,      // transit from ON to OFF
  RADIO_NDEF,       // don't know
  RADIO_TOGGLE
};

// Strings are needed only if you want to output something in Serial Monitor,
// otherwise: save RAM !!
#ifdef debug
const String  radioOnOffStr[6] = {
  "R_OFF",
  "R_ON",
  "R_OFF_TR",     // transit from OFF to ON
  "R_ON_TR",      // transit from ON to OFF
  "R_NDEF",       // don't know
  "R_TOGGLE"
};

const String    RXTXstr[2] = {
"RX", // 0 or false
"TX"  // 1 or true
};
#endif


// modulation mode according to ICOMs documentation
enum radioModMode_t:uint8_t {
  MOD_LSB     = 0,
  MOD_USB,
  MOD_AM,
  MOD_CW,
  MOD_RTTY,
  MOD_FM,
  MOD_WFM,
  MOD_CW_R,
  MOD_RTTY_R,
  MOD_DV,   // 09 (Note: on the ICOM CIV bus, this is coded as 17 in BCD-code, i.e. 0x17)
  MOD_NDEF
};

// clear test translation of the modulation modes
#if defined(debug) && defined(modmode)
const String modModeStr[11] = {
  "LSB   ", // 00 (00 .. 08 is according to ICOM's documentation) 
  "USB   ", // 01
  "AM    ", // 02
  "CW    ", // 03
  "RTTY  ", // 04
  "FM    ", // 05
  "WFM   ", // 06
  "CW-R  ", // 07
  "RTTY-R", // 08
  "DV    ", // 09 (Note: on the ICOM CIV bus, this is coded as 17 in BCD-code, i.e. 0x17)
  "NDEF  "  // 10
};
#endif

// RX filter chosen according to ICOMs documentation
enum radioFilter_t:uint8_t {
  FIL_NDEF    = 0,
  FIL1        = 1,
  FIL2,
  FIL3
};

// clear text translation of the Filter setting
#if defined(debug) && defined(modmode)
const String FilStr[4] = {
  "NDEF",
  "FIL1",   // 1 (1 .. 3 is according to ICOM's documentation)
  "FIL2",
  "FIL3"
};
#endif

//!// put your own defines here if necessary:
// e.g. :


//------------------------------------------------------------------------------------------------------------
// Debugging ...

// Speed of the Serial output messages
#define debugBdRate 115200

// Measure the delay between the incoming PTT line and the outgoing PTT line
#ifdef ESPtriggerMeas
  #define P_Trigger 16
  bool          Trigger=false;
  unsigned long TS_start_delayMeas= millis();
  unsigned long TS_stop_delayMeas = millis();
  #define init_delayMeas pinMode(P_Trigger, INPUT);
  #define set_delayMeas  if (Trigger!=digitalRead(P_Trigger)) {Trigger=digitalRead(P_Trigger);TS_start_delayMeas = millis();}
  #define eval_delayMeas TS_stop_delayMeas = millis();Serial.print("delay: ");Serial.println(TS_stop_delayMeas - TS_start_delayMeas);
#else
  #define init_delayMeas    
  #define set_delayMeas  
  #define eval_delayMeas 
#endif

// time measurement in mikro seconds

#ifdef debug
// for Debugging ...
  unsigned long G_timemarker1;
  unsigned long G_timemarker1a;
  #define SET_TIME_MARKER1 G_timemarker1 = micros();
  #define EVAL_TIME_MARKER1 G_timemarker1a = micros();Serial.print("t1:  ");Serial.println(G_timemarker1a-G_timemarker1);
  #define EVAL_TIME_MARKER1g G_timemarker1a = micros();if ((G_timemarker1a-G_timemarker1)>3000){Serial.print("t1:  ");Serial.println(G_timemarker1a-G_timemarker1);}
#else
  #define SET_TIME_MARKER1
  #define EVAL_TIME_MARKER1
  #define EVAL_TIME_MARKER1g
#endif

#endif // #ifndef defines_h
