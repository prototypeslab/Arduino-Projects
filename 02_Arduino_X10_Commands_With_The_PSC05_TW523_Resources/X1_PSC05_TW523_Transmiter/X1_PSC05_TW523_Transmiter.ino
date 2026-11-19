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

//#include "psc05.h"    // constants for PSC05 X10 Receiver
#define TRANS_PIN      5               // YEL pin 4 of PSC05
#define RCVE_PIN       4               // GRN pin 3 of PSC05
#define ZCROSS_PIN     2               // BLK pin 1 of PSC05
#define LED_PIN        13              // for testing 


//volatile unsigned long mask;           // MSB first - bit 12 - bit 0
//volatile unsigned int X10BitCnt = 0;   // counts bit sequence in frame
//volatile unsigned int ZCrossCnt = 0;   // counts Z crossings in frame
//volatile unsigned long rcveBuff;       // holds the 13 bits received in a frame
//volatile boolean X10rcvd = false;      // true if a new frame has been received
boolean newX10 = false;                // both the unit frame and the command frame received
byte houseCode, unitCode, cmndCode;    // current house, unit, and command code
//byte startCode;                        // only needed for testing - sb B1110 (14)
//volatile unsigned int OFFSET_DELAY;
//volatile unsigned int HALF_CYCLE_DELAY;

//x10 SendX10= x10(ZCROSS_PIN,TRANS_PIN);// set up a x10 library instance:
x10 SendX10= x10(ZCROSS_PIN,TRANS_PIN,RCVE_PIN,LED_PIN);// set up a x10 library instance:

void setup() {
  //attachInterrupt(0,Check_Rcvr,CHANGE);// (pin 2) trigger zero cross
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

  
  //if (newX10)
  {                         // received a new command
    X10_Debug();                       // print out the received command
    newX10 = false;

    //if (unitCode == 1)
    {
      detachInterrupt(0);                  // must detach interrupt before sending
      SendX10.write(HOUSE_A,UNIT_2,RPT_SEND);               
      //if(cmndCode == ON) 
      SendX10.write(HOUSE_A,ON,RPT_SEND); 
      SendX10.write(HOUSE_A,BRIGHT,RPT_SEND); 
      SendX10.write(HOUSE_A,BRIGHT,RPT_SEND); 
      SendX10.write(HOUSE_A,BRIGHT,RPT_SEND); 
      SendX10.write(HOUSE_A,BRIGHT,RPT_SEND); 
      digitalWrite(LED_PIN, LOW);
      delay(1000);
      SendX10.write(HOUSE_A,DIM,RPT_SEND); 
      SendX10.write(HOUSE_A,DIM,RPT_SEND);
      SendX10.write(HOUSE_A,DIM,RPT_SEND);
      SendX10.write(HOUSE_A,DIM,RPT_SEND);
      //if(cmndCode == OFF) 
      SendX10.write(HOUSE_A,OFF,RPT_SEND);  
      //attachInterrupt(0,Check_Rcvr,CHANGE);// re-attach interrupt
 
    }
  }
  
} 
/*
void Check_Rcvr(){    // ISR - called when zero crossing (on CHANGE)
  if (X10BitCnt == 0) {                // looking for new frame
    delayMicroseconds(OFFSET_DELAY);   // wait for bit
    if(digitalRead(RCVE_PIN)) return;  // still high - no start bit - get out
    digitalWrite(LED_PIN, HIGH);       // indicate you got something
    rcveBuff = 0;
    mask = 0x1000;                     // bitmask with bit 12 set
    rcveBuff = rcveBuff | mask;        // sets bit 12 (highest)
    mask = mask >> 1;                  // move bit down in bit mask
    X10BitCnt = 1;                     // inc the bit count
    ZCrossCnt = 1;                     // need to count zero crossings too
    return;
  }
  // Begins here if NOT the first bit . . .
  ZCrossCnt++;                         // inc the zero crossing count
  // after SC (first 4 bits) ignore the pariety bits - so only read odd crossings
  if (X10BitCnt < 5 || (ZCrossCnt & 0x01)){ // if it's an odd # zero crossing
    delayMicroseconds(OFFSET_DELAY);   // wait for bit
    if(!digitalRead(RCVE_PIN)) rcveBuff = rcveBuff | mask;  // got a 1 set the bit, else skip and leave it 0
    mask = mask >> 1;                  // move bit down in bit mask
    X10BitCnt++;

    if(X10BitCnt == 13){               // done with frame after 13 bits
      for (byte i=0;i<5;i++)delayMicroseconds(HALF_CYCLE_DELAY); // need this
      X10rcvd = true;                  // a new frame has been received
      digitalWrite(LED_PIN, LOW);
      X10BitCnt = 0;
      //Parse_Frame();                   // parse out the house & unit code and command
    }
  }
}

void Parse_Frame() {   // parses the receive buffer to get House, Unit, and Cmnd
  if(rcveBuff & 0x1){                  // last bit set so it's a command
    cmndCode = rcveBuff & 0x1F;        // mask 5 bits 0 - 4 to get the command
    newX10 = true;                     // now have complete pair of frames
  }
  else {                               // last bit not set so it's a unit
    unitCode = rcveBuff & 0x1F;        // mask 5 bits 0 - 4 to get the unit
    newX10 = false;                    // now wait for the command
    for (byte i=0; i<16; i++){         // use lookup table to get the actual unit #
      if (Unit[i] == unitCode){
        unitCode = i+1;                // this gives Unit 1-16
        break;                         // stop search when found!
      }
    }
  }
  rcveBuff = rcveBuff >> 5;            // shift the house code down to LSB
  houseCode = rcveBuff & 0x0F;         // mask the last 4 bits to get the house code
  for (byte i=0; i<16; i++){           // use lookup table to get the actual command #
    if (House[i] == houseCode){ 
      houseCode = i+65;                // this gives House 'A' - 'P'
      break;                           // stop search when found!
    }
  }
  rcveBuff = rcveBuff >> 4;            // shift the start code down to LSB
  startCode = rcveBuff & 0x0F;         // mask the last 4 bits to get the start code
  X10rcvd = false;                     // reset status
}
*/
void X10_Debug(){
  //Serial.print("SC-");
 // Serial.print(startCode,BIN);
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
