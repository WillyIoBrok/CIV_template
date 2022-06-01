/* 
CIV_template - z_userprog

28/05/22

Adapted for the functionality which the project of Rainer, Dk1RS, required
i.e. set the Filters in the PA "HVLA1K3" depending on the frequency info of the IC705
via 4 digital lines (binary coded)

The calls to this user programs shall be inserted wherever it suits - search for //!//
in all files

*/

//=========================================================================================
// user part of the defines

#define VERSION_USER "usrprg DK1RS V0_3 June 1st, 2022"


#define NUM_BANDS 11   /* Number of Bands (depending on the radio) */

// if defined, the bit pattern of the output pins is inverted in order to compensate
// the effect of inverting HW drivers (active, i.e.uncommented by default)
#define invDriver       //if active, inverts band BCD out
#define inv_PTT         //if active, PTT out is Low going.

// Mapping of portpins to function ===========================================================================

// for ESP32!
// digital control lines of LPF
#define P_BCD0            33
#define P_BCD1            25
#define P_BCD2            26
#define P_BCD3            27

//PTT out pin
#define P_PTT             12

//=========================================================================================
// user part of the database

uint8_t currentBand = NUM_BANDS;  // NUM_BANDS means "not defined"
uint8_t currentBCDsetting = 0x00; // 0x00 == undefined

//=========================================================================================
// handle the frequency info and set the HW

//-----------------------------------------------------------------------------------------
// tables for band selection and bittpattern calculation

// lower limits[kHz] of the bands:
constexpr unsigned long lowlimits[NUM_BANDS] = {
  1791, 3491, 5291, 6991,  9991, 13991, 18051, 20991, 24881, 27991, 49991
};
// upper limits[kHz] of the bands:
constexpr unsigned long uplimits[NUM_BANDS] = {
  2100, 4000, 5400, 7500, 10200, 14500, 18200, 21600, 25000, 29800, 54100
};

constexpr uint8_t band2BCD [NUM_BANDS+1] = { 
// 160    80    60    40     30     20     17     15     12     10      6  NDEF
  0x01, 0x02, 0x03, 0x03,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0A, 0x00
};

//------------------------------------------------------------
// set the bitpattern in the HW

void set_HW (uint8_t BCDsetting) {

  digitalWrite  (P_BCD0, ( BCDsetting     & 0b00000001));
  digitalWrite  (P_BCD1, ((BCDsetting>>1) & 0b00000001));
  digitalWrite  (P_BCD2, ((BCDsetting>>2) & 0b00000001));
  digitalWrite  (P_BCD3, ((BCDsetting>>3) & 0b00000001));

#ifdef debug
  // Test output to control the proper functioning:
  Serial.print ("Pins ");
  Serial.print (P_BCD3); Serial.print (' '); Serial.print (P_BCD2); Serial.print (' ');
  Serial.print (P_BCD1); Serial.print (' '); Serial.print (P_BCD0); Serial.print (" : "); 
  Serial.print (((BCDsetting>>3) & 0b00000001),BIN);
  Serial.print (' ');
  Serial.print (((BCDsetting>>2) & 0b00000001),BIN);
  Serial.print (' ');
  Serial.print (((BCDsetting>>1) & 0b00000001),BIN);
  Serial.print (' ');
  Serial.print (( BCDsetting     & 0b00000001),BIN);
  Serial.println (' ');
#endif

}

//-----------------------------------------------------------------------------------------
// get the bandnumber matching to the frequency (in kHz)

byte get_Band(unsigned long frq){
  byte i;
  for (i=0; i<NUM_BANDS; i++) {
    if ((frq >= lowlimits[i]) && (frq <= uplimits[i])){
      return i;
    }
  }
  return NUM_BANDS; // no valid band found -> return not defined
}

//------------------------------------------------------------
// process the frequency received from the radio

void set_PAbands(unsigned long frequency) {
  unsigned long freq_kHz;

  freq_kHz = frequency/1000;              // frequency is now in kHz
  currentBand = get_Band(freq_kHz);       // get band according the current frequency

#ifdef debug
  // Test-output to serial monitor:
  Serial.print("Frequency: ");  Serial.print(freq_kHz);
  Serial.print("  Band: ");     Serial.print(currentBand);  
  Serial.print("  BCD: ");      Serial.println(band2BCD[currentBand],BIN);  
#endif

  // load the bitpattern into the HW:
  // currentBand : 0 ... NUM_BANDS
  // BCD :         0 ... 0b00001010 ( == 0x0A )
  
  // "~" inverts the bitpattern!  (0 -> 1 ; 1 -> 0)
  // this can be used to compensate the effect of inverting HW buffers

#ifdef invDriver 
  set_HW ( ~ band2BCD[currentBand] );
#else
  set_HW (   band2BCD[currentBand] );
#endif

}


//=========================================================================================
// this is called, when the RX/TX state changes ...
void  userPTT(uint8_t newState) {

  #ifdef Inv_PTT 
    digitalWrite (P_PTT,  !newState);   //--inverted-- output version:  Clr = Tx, Hi = Rx  
  #else
    digitalWrite (P_PTT,  newState);    // Clr = Rx, Hi = Tx
  #endif 
  
}

//=========================================================================================
// this is called, whenever there is new frequency information ...
void userFrequency(unsigned long newFrequency) {
	set_PAbands(newFrequency);
}

//=========================================================================================
// this will be called in the setup after startup
void  userSetup(){

  Serial.println (VERSION_USER);

  // initialize the PTT-pin as RX !
	pinMode (P_PTT, OUTPUT);
  userPTT(0);

  // set the used HW pins (see defines.h!) as output and set it to 0V (at the Input of the PA!!) initially
  pinMode       (P_BCD0, OUTPUT);
  pinMode       (P_BCD1, OUTPUT);
  pinMode       (P_BCD2, OUTPUT);
  pinMode       (P_BCD3, OUTPUT);

#ifdef invDriver 
  digitalWrite  (P_BCD0, HIGH);
  digitalWrite  (P_BCD1, HIGH);
  digitalWrite  (P_BCD2, HIGH);
  digitalWrite  (P_BCD3, HIGH);
#else
  digitalWrite  (P_BCD0, LOW);
  digitalWrite  (P_BCD1, LOW);
  digitalWrite  (P_BCD2, LOW);
  digitalWrite  (P_BCD3, LOW);
#endif

}

//-------------------------------------------------------------------------------------
// this will be called in the baseloop every BASELOOP_TICK[ms]
void  userBaseLoop() {
  
}
