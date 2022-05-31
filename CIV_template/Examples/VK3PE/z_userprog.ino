/* 
CIV_template - z_userprog - adapted for the requirements of Glenn, VK3PE by DK8RW, May 16, 22
    vk3pe is using a "TTGO" ESP32 module.
    Band select BCD outputs set to be active Hi.NOTE: a BCD to Decimal chip will be used also
     to provide 10 band outputs.
    PTT output is active LOW

This is the part, where the user can put his own procedures in

The calls to this user programs shall be inserted wherever it suits - search for //!//
in all files

*/

#include <TFT_eSPI.h>       //using this LIB now.  https://github.com/Bodmer/TFT_eSPI    
                            // IMPORTANT!  
                            //      In the "User_Setup_Select.h" file, enable "#include <User_Setups/Setup25_TTGO_T_Display.h>"

//=========================================================================================
// user part of the defines

// if defined, the bit pattern of the output pins is inverted in order to compensate
// the effect of inverting HW drivers (active, i.e.uncommented by default)
#define invDriver         //if active, inverts band BCD out
#define Inv_PTT           //if active, PTT out is Low going.


#define VERSION_USER "usrprg VK3PE V0_3 May 31st, 2022"

#define NUM_BANDS 10   /* Number of Bands (depending on the radio) */

//-----------------------------------------------------------------------------------------
//for TFT
TFT_eSPI tft = TFT_eSPI();

#define screen_width  240       //placement of text etc must fit withing these boundaries.
#define screen_heigth 135
 
// all my known colors for ST7789 TFT (but not all used in program)
#define B_DD6USB 0x0004    //   0,   0,   4  my preferred background color !!!   now vk3pe ?
#define BLACK 0x0000       //   0,   0,   0
#define NAVY 0x000F        //   0,   0, 123
#define DARKGREEN 0x03E0   //   0, 125,   0
#define DARKCYAN 0x03EF    //   0, 125, 123
#define MAROON 0x7800      // 123,   0,   0
#define PURPLE 0x780F      // 123,   0, 123
#define OLIVE 0x7BE0       // 123, 125,   0
#define LIGHTGREY 0xC618   // 198, 195, 198
#define DARKGREY 0x7BEF    // 123, 125, 123
#define BLUE 0x001F        //   0,   0, 255
#define GREEN 0x07E0       //   0, 255,   0
#define CYAN 0x07FF        //   0, 255, 255
#define RED 0xF800         // 255,   0,   0
#define MAGENTA 0xF81F     // 255,   0, 255
#define YELLOW 0xFFE0      // 255, 255,   0
#define WHITE 0xFFFF       // 255, 255, 255
#define ORANGE 0xFD20      // 255, 165,   0
#define GREENYELLOW 0xAFE5 // 173, 255,  41
#define PINK 0xFC18        // 255, 130, 198
//*************************************************************

//=================================================
// Mapping of port-pins to functions on ESP32 TTGO
//=================================================

// the Pins for SPI
#define TFT_CS    5
#define TFT_DC   16
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_RST  23
#define TFT_BL    4

#define PTTpin    17      //PTT out pin

#define P_BCD0    33      //now allocate BANDS OUT
#define P_BCD1    25
#define P_BCD2    26
#define P_BCD3    27


//=========================================================================================
// user part of the database
// e.g. :
uint8_t         G_currentBand = NUM_BANDS;  // Band in use (default: not defined)



//=====================================================
// this is called, when the RX/TX state changes ...
//=====================================================
void  userPTT(uint8_t newState) {           

#ifdef debug
   Serial.println (newState);                     //prints '1' for Tx, '0' for Rx
#endif
  tft.setFreeFont  ( &FreeSansBold9pt7b );        //previous setup text was smaller.
  //tft.setTextColor(WHITE) ;

   
if (newState){                                    // '1' = Tx mode
  tft.setCursor(185, 95) ;   
  tft.fillRect(182, 60, 50, 50, RED) ;            //Erase Rx mode.  vk3pe x,y,width,height,colour 10,40,137,40  
  tft.setTextColor(WHITE) ;
  tft.print("Tx"); 
}   //Tx mode
else {
  tft.setCursor(183, 95) ; 
  tft.fillRect(182, 60, 50, 50, GREEN) ;          //erase Tx mode:    location x, y, width, height,  colour
  tft.setTextColor(BLUE) ;                        // Better contrast
  tft.print("Rx") ;
} //Rx mode

#ifdef Inv_PTT 
  digitalWrite (PTTpin,  !newState);    //--inverted-- output version:  Clr =Tx, Hi =Rx  
  #else
  digitalWrite (PTTpin,  newState);    // Clr =Rx, Hi =Tx   
#endif 
}

//=========================================================================================
// creating bandinfo based on the frequency info

//-----------------------------------------------------------------------------------------
// tables for band selection and bittpattern calculation

//for IC-705 which has no 60M band:
//---------------------------------
// !!! pls adapt "NUM_BANDS" if changing the number of entries in the tables below !!!

// lower limits[kHz] of the bands: NOTE, these limits may not accord with band  edges in your country.
constexpr unsigned long lowlimits[NUM_BANDS] = {
  1791, 3491,  6991,  9991, 13991, 18051, 20991, 24881, 27991, 49991
};
// upper limits[kHz] of the bands:  //see NOTE above.
constexpr unsigned long uplimits[NUM_BANDS] = {
  2100, 4000,  7500, 10200, 14500, 18200, 21600, 25000, 29800, 54100
};

constexpr uint8_t band2BCD [NUM_BANDS+1] = {            //
  
// 160    80        40     30     20     17     15     12     10      6  NDEF
  //0x01, 0x02,     0x03,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0A, 0x00

  //-----------------------------------------------------------------------------------------
  // use these if want to use 160M to 6M band select outputs using 74HC32 BCD to Dec decoder.
  // ie 160M will activate pin 1 on the 74HC42, etc to pin9 for 6M
  //-----------------------------------------------------------------------------------------
  // 160    80        40     30     20     17     15     12     10      6  NDEF
  0x00, 0x01,     0x02,  0x03,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09, 0x00

};

// "xxM" display for the TFT display. ie show what band the unit is current on in "meters"
const String (band2string [NUM_BANDS+1]) = {
// 160     80       40     30      20     17      15     12     10      6     NDEF
  "160m"," 80m",  " 40m"," 30m", " 20m"," 17m", " 15m"," 12m"," 10m","  6m", "Out"

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
  Serial.print (" Pins ");
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
  //for (i=1; i<NUM_BANDS; i++) {   
    if ((frq >= lowlimits[i]) && (frq <= uplimits[i])){
      return i;
    }
  }
  return NUM_BANDS; // no valid band found -> return not defined
}

//------------------------------------------------------------------
//    Show frequency in 'kHz' and band in 'Meters' text on TFT vk3pe
//------------------------------------------------------------------
void show_Meters(void)    
{

  // Show Freq[KHz]
  tft.setCursor(10, 120) ;                //- 
  tft.fillRect(10, 90, 125, 40, BLUE) ;   //-erase   x,y,width, height 
  tft.setTextColor(WHITE) ;               //-
  tft.print (band2string[G_currentBand]); //-



}

//------------------------------------------------------------
// process the frequency received from the radio
//------------------------------------------------------------

void set_PAbands(unsigned long frequency) {
  unsigned long freq_kHz;

  freq_kHz = G_frequency/1000;            // frequency is now in kHz
  G_currentBand = get_Band(freq_kHz);     // get band according the current frequency

  tft.setFreeFont  ( &FreeSansBold9pt7b ); //bigger numbers etc from now on. <<<<<<<<-------------------
  tft.setCursor(10, 75) ;                 // for bigger print size
                                          //already use white from previous :-
  tft.fillRect(10, 40, 125, 40, BLUE) ;   //erase previous freq   vk3pe x,y,width,height,colour 10,40,137,40
  tft.setTextColor (WHITE);               // at power up not set!

  
  tft.print(frequency/1000) ;             //show Frequency in KHz

    
#ifdef debug
  // Test-output to serial monitor:
  Serial.print("Frequency: ");  Serial.print(freq_kHz);
  Serial.print("  Band: ");     Serial.print(G_currentBand); 
  Serial.println(band2string[G_currentBand]);
  Serial.print("BCD: ");      Serial.print(band2BCD[G_currentBand],BIN); 
#endif

  // load the bitpattern into the HW:
  // currentBand : 0 ... NUM_BANDS
  // BCD :         0 ... 0b00001010 ( == 0x0A )
  
  // "~" inverts the bitpattern!  (0 -> 1 ; 1 -> 0, e.g. 0b11001101 -> 0b00110010)
  // this can be used to compensate the effect of inverting HW buffers

#ifdef invDriver
  set_HW (   band2BCD[G_currentBand] );   //inverts 4 bit BCD out
#else
  set_HW ( ~ band2BCD[G_currentBand] );
#endif

  show_Meters();            //Show frequency in kHz and band in Meters (80m etc) on TFT
}

//=========================================================================================
// this is called, whenever there is new frequency information ...
// this is available in the global variable "G_frequency" ...
void userFrequency(unsigned long newFrequency) {

  set_PAbands(G_frequency);

}

//-----------------------------------------------------------------------------------------
// initialise the TFT display
//-----------------------------------------------------------------------------------------

void init_TFT(void)
{
  //tft.init(screen_heigth, screen_width) ;  //not used

  tft.init();
  pinMode(TFT_BL, OUTPUT) ;
  digitalWrite(TFT_BL, HIGH) ;              // switch backlight on
  
  tft.fillScreen(BLUE) ;
  tft.setRotation(1) ;
  tft.fillRoundRect(0, 0, tft.width(), 30, 5, MAROON) ;   // background for screen title
  tft.drawRoundRect(0, 0, tft.width(), 30, 5, WHITE) ;    //with white border.
  
  tft.setTextSize(2) ;                  //for default Font only.Font is later changed.
  tft.setTextColor(YELLOW) ;
  tft.setCursor(5, 10) ;                //top line
  tft.print("IC705 BT interface") ;
  
  tft.setTextColor( WHITE) ;            //white from now on
  
  tft.setCursor(135, 60);               //
  tft.print("kHz") ;
  //tft.setCursor(150, 95) ; 
  tft.setCursor(135, 107) ;             //
  tft.print("band") ;                   //"160m" etc   or Out if invalid Freq. for Ham bands.
}

//=========================================================================================
// this will be called in the setup after startup
void  userSetup(){

  Serial.println (VERSION_USER);

  // set the used HW pins (see defines.h!) as output and set it to 0V (at the Input of the PA!!) initially

  pinMode       (PTTpin, OUTPUT);     //PTT out pin 
  digitalWrite  (PTTpin, HIGH);       //set 'Rx mode' > high
  pinMode       (P_BCD0, OUTPUT);
  pinMode       (P_BCD1, OUTPUT);
  pinMode       (P_BCD2, OUTPUT);
  pinMode       (P_BCD3, OUTPUT);

#ifdef invDriver 
  digitalWrite  (P_BCD0, LOW);
  digitalWrite  (P_BCD1, LOW);
  digitalWrite  (P_BCD2, LOW);
  digitalWrite  (P_BCD3, LOW);
#else
  digitalWrite  (P_BCD0, HIGH);
  digitalWrite  (P_BCD1, HIGH);
  digitalWrite  (P_BCD2, HIGH);
  digitalWrite  (P_BCD3, HIGH);
#endif

  init_TFT();

  userPTT(0);  // initialize the "RX" symbol in the screen

}

//-------------------------------------------------------------------------------------
// this will be called in the baseloop every BASELOOP_TICK[ms]
void  userBaseLoop() {
  
}
