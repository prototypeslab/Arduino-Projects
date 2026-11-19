//
// TCP005: A simple server... not a web server... for Arduino..
//    An expanded TCP004, which illustrates the core elements.
//    Requires purpose written client software.... see below.

// See http://sheepdogguides.com/arduino/art5simpsrv.htm for details

//Version: 11 Oct 2011 (with some tweaks done on webpage)
//N.B. THE FOLLOWING ALSO "define" the version. And they can
//   be read by the client, with command $7E.
//These MUST be set to printable ASCII, e.g 'A' or 65, as
//   the client software expects this.
//The checksum can be in the range 0-255
static uint8_t bVerMostSB='0';
static uint8_t bVerMidSB='0';
static uint8_t bVerLSB='1';

//    Requires purpose-crafted client software.
//    TCP002.exe in....
//    http://sheepdogguides.com/zips/TCP001.zip
//    ... is suitable. for accessing the BB 82
//        command and (static) response.
//    If you know of, or have written, Linux
//    equivalents, please contact http://sheepdogguides.com/
//
// This code is in the public domain.
//
// It requires a W5100 based ethernet interface, although converting
//    the software to run with a Microchip ENC28J60 should not be hard.

//To do: Add "read 1-Wire"... and make client s/w dongleable via 1-Wire chip?

#include <SPI.h>
#include <Ethernet.h>

//The following determine the MAC address and IP address
//   that will be in the ethernet shield on your Arduino.
//Both must be unique on your LAN

uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
uint8_t ip[] = { 10,0,1,55 };
//192.168.1.2
//uint8_t ip[] = { 192,168,1,55 };

//The following determines what port the server will listen on.
int iSPort = 5202;

EthernetServer server(iSPort);

boolean boLEDOn;//To do with non-essential LED on/off stuff
uint8_t bLEDpin=8;//To do with non-essential LED on/off stuff
  //Software assumes an LED-with-resistor on this pin. See LEDon,
  //LEDoff subroutines.
  //(Pin 13 unsuitable, for some reason, when using Ethernet Pro hardware)
uint8_t bBeepPin=7;//Pin a sounder will be connected to. Pin
  //high: Beep on, low: off.
uint8_t bBeepDur=10;
  //Length of beep, in milliseconds. Do not make this longer
  //  than you can help. The system is blocked while the beep
  //  is on. (Too short, and the sounder can't sound, though.)
//N.B.The pins defined by bLEDpin and bBeepPin will be configured as an output...
  //be careful to match your program with your hardware!

int iCommandsAccepted;//Where we count how many commands have come in,
// or things that we thought might be commands...

uint8_t response[68];
// N.B. Size of response limited by what you put in prior line.
// Be careful not to overrun the buffer.
uint8_t bRespLen;

boolean boValidCommand;

void Beep(){
digitalWrite(bBeepPin,HIGH);
delay(bBeepDur);
digitalWrite(bBeepPin,LOW);
};

void LEDOn(){
//Note that this makes the assumption that the LED+resistor
//  go to GND from the output pin controlling them
//Just change "HIGH" to "LOW" if your LED is wired the other
//  way.
    digitalWrite(bLEDpin,HIGH);
    boLEDOn=true;
}

void LEDOff(){
//Note that this makes the assumption that the LED+resistor
//  go to GND from the output pin controlling them
//Just change "LOW" to "HIGH" if your LED is wired the other
//  way.
    digitalWrite(bLEDpin,LOW);
    boLEDOn=false;
}

void ProcessBB82(){
      uint8_t *r = response;
      *r++ = 0xBB;
      *r++ = 0x82;
      *r++ = 0x01;
      *r++ = 0x01;
      *r++ = 0xFF;
      *r++ = 0xFF;
      bRespLen=6;
      boValidCommand=true;
}

void Process7E81(){
// Report Version information to client.
//   This routine as yet untested, as a way to call
//   it does not exist in the client software

      uint8_t *r = response;
      *r++ = 0x7E;
      *r++ = 0x54;
      *r++ = bVerMostSB;
      *r++ = bVerMidSB;
      *r++ = bVerLSB;
      *r++ = bVerMidSB ^ bVerLSB;//XOR the bytes (checksum)
      bRespLen=6;
      boValidCommand=true;
}

void Process7F80(){
      uint8_t bMS=iCommandsAccepted >> 8;//MSB of iCommandsAccepted
      uint8_t bLS=iCommandsAccepted & 0xFF;//LSB of iCommandsAccepted

      uint8_t *r = response;
      *r++ = 0x7F;
      *r++ = 0x55;
      *r++ = bMS;
      *r++ = bLS;
      *r++ = bMS ^ bLS;//XOR the bytes (checksum)
      bRespLen=5;
      boValidCommand=true;
}

void Process50AF(){
      LEDOff();
      uint8_t *r = response;
      *r++ = 0x50;
      *r++ = 0x00;
      *r++ = 0x05;
      bRespLen=3;
      boValidCommand=true;
}

void Process51AE(){
      LEDOn();
      uint8_t *r = response;
      *r++ = 0x51;
      *r++ = 0x00;
      *r++ = 0x04;
      bRespLen=3;
      boValidCommand=true;
}

void Process718E(){
      uint16_t iAn1=analogRead(1);
      uint8_t bMS=iAn1 >> 8;//MSB of iAn1
      uint8_t bLS=iAn1 & 0xFF;//LSB of iAn1

      uint8_t *r = response;
      *r++ = 0x71;
      *r++ = 0x51;
      *r++ = bMS;
      *r++ = bLS;
      *r++ = bMS ^ bLS;//XOR of data bytes, "checksum"
      bRespLen=5;
      boValidCommand=true;
}


void setup(){
  iCommandsAccepted=0;
  pinMode(bLEDpin,OUTPUT);
  LEDOn();//Also makes boLEDOn true.
  pinMode(bBeepPin,OUTPUT);
  digitalWrite(bBeepPin,LOW);

  // Initialize the W5100 ethernet chip
  Ethernet.begin(mac, ip);

  // Start the serial library (for debug output)
  // The serial monitor is not required for the server's operation.
  Serial.begin(9600);

  // Start the TCP server
  Serial.print("Opening server...\n");
  server.begin();
}

void loop(){
  EthernetClient c = server.available();
  if (c.connected()) {
    Serial.print("Incoming connection!\n");
    // Read two bytes from client
    uint8_t b1 = c.read();
    uint8_t b2 = c.read();
    ++iCommandsAccepted;

    // Debugging: Show me what the client sent
    Serial.print("The client said 0x");
    Serial.print(b1,HEX);
    Serial.print(" 0x");
    Serial.print(b2,HEX);

    Serial.print("   ");
    Serial.print(iCommandsAccepted);

    Serial.print("\n");

    // Deal with the command...
    boValidCommand=false;//Set true within a command processor, if one found
    if ((b1 == 0x50) and (b2==0xAF)) Process50AF(); 
    if ((b1 == 0x51) and (b2==0xAE)) Process51AE(); 
    if ((b1 == 0x71) and (b2==0x8E)) Process718E();
    if ((b1 == 0x7E) and (b2==0x81)) Process7E81();
    if ((b1 == 0x7F) and (b2==0x80)) Process7F80();
    if ((b1 == 0xBB) and (b2==0x82)) ProcessBB82();
    //Test only
    if ((b1 == 0x41) and (b2==0x41)) Process50AF(); //PR  52
    if ((b1 == 0x41) and (b2==0x42)) Process51AE(); //QS  53
    // Send response
    if (boValidCommand) {
          c.write(response,bRespLen);
          Beep();//It would be possible to move this
            //to within the handlers of selected
            //commands, if you did not want a beep
            //for EVERY command processed.
            //Or, elsewhere, it would report all
            //requests for service, whether valid or not.
          }

    };//End of if c.connected (then...) block
    // Close the connection... TKB: close connection or stop server? Don't want latter!
    //TKB REMMED OUT, as CONNECTION REPEATEDLY CLOSED...
    //c.stop();
    //seems to work! (Didn't (properly) with c.stop... server shut down?
 }//end of Loop()
