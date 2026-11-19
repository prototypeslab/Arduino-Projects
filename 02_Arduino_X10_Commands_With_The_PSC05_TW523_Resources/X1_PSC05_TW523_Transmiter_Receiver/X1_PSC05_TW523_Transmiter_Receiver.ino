//Working

/* Arduino Interface to the PSC05 X10 Receiver.                       BroHogan 3/24/09
 * SETUP: X10 PSC05/TW523 RJ11 to Arduino (timing for 60Hz)
 * - RJ11 pin 1 (BLK) -> Pin 2 (Interrupt 0) = Zero Crossing
 * - RJ11 pin 2 (RED) -> GND
 * - RJ11 pin 3 (GRN) -> Pin 4 = Arduino receive
 * - RJ11 pin 4 (YEL) -> Pin 5 = Arduino transmit (via X10 Lib)
 * NOTES:
 * - Must detach interrup when transmitting with X10 Lib 
 */

#include "Arduino.h"//"WProgram.h"                  // this is needed to compile with Rel. 0013
#include <x10.h>                       // X10 lib is used for transmitting X10
#include <x10constants.h>              // X10 Lib constants
#define RPT_SEND 2                     // how many times transmit repeats if noisy set higher

//#include "psc05b.h"    // constants for PSC05 X10 Receiver
#define TRANS_PIN      5               // YEL pin 4 of PSC05
#define RCVE_PIN       4               // GRN pin 3 of PSC05
#define ZCROSS_PIN     2               // BLK pin 1 of PSC05
#define LED_PIN        13              // for testing 

byte HouseCode[16] = {              // Lookup table for House Code
  B0110,  // A
  B1110,  // B
  B0010,  // C
  B1010,  // D
  B0001,  // E
  B1001,  // F
  B0101,  // G
  B1101,  // H
  B0111,  // I
  B1111,  // J
  B0011,  // K
  B1011,  // L
  B0000,  // M
  B1000,  // N
  B0100,  // O
  B1100,  // P
};
byte UnitCode[16] = {               // Lookup table for Unit Code
  B01100,  // 1
  B11100,  // 2
  B00100,  // 3
  B10100,  // 4
  B00010,  // 5
  B10010,  // 6
  B01010,  // 7
  B11010,  // 8
  B01110,  // 9
  B11110,  // 10
  B00110,  // 11
  B10110,  // 12
  B00000,  // 13
  B10000,  // 14
  B01000,  // 15
  B11000,  // 16
};

volatile unsigned long _mask;           // MSB first - bit 12 - bit 0
volatile unsigned int _X10BitCnt = 0;   // counts bit sequence in frame
volatile unsigned int _ZCrossCnt = 0;   // counts Z crossings in frame
volatile unsigned long _rcveBuff;       // holds the 13 bits received in a frame
volatile boolean _X10rcvd = false;      // true if a new frame has been received
boolean newX10 = false;                // both the unit frame and the command frame received
byte houseCode, unitCode, cmndCode;    // current house, unit, and command code
byte _startCode; 

// only needed for testing - sb B1110 (14)
//volatile unsigned int OFFSET_DELAY;
//volatile unsigned int HALF_CYCLE_DELAY;

//x10 SendX10= x10(ZCROSS_PIN,TRANS_PIN);// set up a x10 library instance:
x10 SendX10= x10(ZCROSS_PIN,TRANS_PIN,RCVE_PIN,LED_PIN);// set up a x10 library instance:

void setup() {
  attachInterrupt(0,Check_Rcvr,CHANGE);// (pin 2) trigger zero cross
  Serial.begin(9600);
  pinMode(LED_PIN,OUTPUT);             // onboard LED
  pinMode(RCVE_PIN,INPUT);             // receive X10 commands - low = 1
  pinMode(ZCROSS_PIN,INPUT);           // zero crossing - 60 Hz square wave
  digitalWrite(RCVE_PIN, HIGH);        // set 20K pullup (low active signal)
  digitalWrite(ZCROSS_PIN, HIGH);      // set 20K pullup (low active signal)
}

void loop(){
      digitalWrite(LED_PIN, HIGH);
      delay(1000);

  
  if (newX10)
  {                         // received a new command
    X10_Debug();                       // print out the received command
    newX10 = false;

    if (unitCode == 5)
    {
      detachInterrupt(0);                  // must detach interrupt before sending
      SendX10.write(HOUSE_P,UNIT_5,RPT_SEND);               
      if(cmndCode == ON)
     { 
      SendX10.write(HOUSE_P,ON,RPT_SEND); 
      SendX10.write(HOUSE_P,BRIGHT,RPT_SEND); 
      SendX10.write(HOUSE_P,BRIGHT,RPT_SEND); 
      SendX10.write(HOUSE_P,BRIGHT,RPT_SEND); 
      SendX10.write(HOUSE_P,BRIGHT,RPT_SEND); 
     
      digitalWrite(LED_PIN, LOW);
      delay(1000);
      SendX10.write(HOUSE_P,DIM,RPT_SEND); 
      SendX10.write(HOUSE_P,DIM,RPT_SEND);
      SendX10.write(HOUSE_P,DIM,RPT_SEND);
      SendX10.write(HOUSE_P,DIM,RPT_SEND);
    }
      if(cmndCode == OFF) 
      {
      SendX10.write(HOUSE_P,OFF,RPT_SEND);  
      }
      attachInterrupt(0,Check_Rcvr,CHANGE);// re-attach interrupt
 
    }

  }
            digitalWrite(LED_PIN, LOW);
      delay(1000);
} 

void Check_Rcvr(){    // ISR - called when zero crossing (on CHANGE)
  if (_X10BitCnt == 0) {                // looking for new frame
    delayMicroseconds(OFFSET_DELAY);   // wait for bit
    if(digitalRead(RCVE_PIN)) return;  // still high - no start bit - get out
    digitalWrite(LED_PIN, HIGH);       // indicate you got something
    _rcveBuff = 0;
    _mask = 0x1000;                     // bitmask with bit 12 set
    _rcveBuff = _rcveBuff | _mask;        // sets bit 12 (highest)
    _mask = _mask >> 1;                  // move bit down in bit mask
    _X10BitCnt = 1;                     // inc the bit count
    _ZCrossCnt = 1;                     // need to count zero crossings too
    return;
  }
  // Begins here if NOT the first bit . . .
  _ZCrossCnt++;                         // inc the zero crossing count
  // after SC (first 4 bits) ignore the pariety bits - so only read odd crossings
  if (_X10BitCnt < 5 || (_ZCrossCnt & 0x01)){ // if it's an odd # zero crossing
    delayMicroseconds(OFFSET_DELAY);   // wait for bit
    if(!digitalRead(RCVE_PIN)) _rcveBuff = _rcveBuff | _mask;  // got a 1 set the bit, else skip and leave it 0
    _mask = _mask >> 1;                  // move bit down in bit mask
    _X10BitCnt++;

    if(_X10BitCnt == 13){               // done with frame after 13 bits
      for (byte i=0;i<5;i++)delayMicroseconds(HALF_CYCLE_DELAY); // need this
      _X10rcvd = true;                  // a new frame has been received
      digitalWrite(LED_PIN, LOW);
      _X10BitCnt = 0;
      Parse_Frame();                   // parse out the house & unit code and command
    }
  }
}

void Parse_Frame() {   // parses the receive buffer to get House, Unit, and Cmnd
  if(_rcveBuff & 0x1){                  // last bit set so it's a command
    cmndCode = _rcveBuff & 0x1F;        // mask 5 bits 0 - 4 to get the command
    newX10 = true;                     // now have complete pair of frames
  }
  else {                               // last bit not set so it's a unit
    unitCode = _rcveBuff & 0x1F;        // mask 5 bits 0 - 4 to get the unit
    newX10 = false;                    // now wait for the command
    for (byte i=0; i<16; i++){         // use lookup table to get the actual unit #
      if (UnitCode[i] == unitCode){
        unitCode = i+1;                // this gives Unit 1-16
        break;                         // stop search when found!
      }
    }
  }
  _rcveBuff = _rcveBuff >> 5;            // shift the house code down to LSB
  houseCode = _rcveBuff & 0x0F;         // mask the last 4 bits to get the house code
  for (byte i=0; i<16; i++){           // use lookup table to get the actual command #
    if (HouseCode[i] == houseCode){ 
      houseCode = i+65;                // this gives House 'A' - 'P'
      break;                           // stop search when found!
    }
  }
  _rcveBuff = _rcveBuff >> 4;            // shift the start code down to LSB
  _startCode = _rcveBuff & 0x0F;         // mask the last 4 bits to get the start code
  _X10rcvd = false;                     // reset status
}

void X10_Debug(){
  Serial.print("SC-");
  Serial.print(_startCode,BIN);
  Serial.print(" HOUSE-");
  Serial.print(houseCode);
  Serial.print(" UNIT-");
  Serial.print(unitCode,DEC);
  Serial.print(" CMND");
  Serial.print(cmndCode,DEC);
  if(cmndCode == ON)Serial.print(" (ON)");
  if(cmndCode == OFF)Serial.print(" (OFF)");
  Serial.println("");
}
