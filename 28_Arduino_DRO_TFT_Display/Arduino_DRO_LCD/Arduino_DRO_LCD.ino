
/*
 Font draw speed and flicker test, draws all numbers 0-999 in each font
 Average time in milliseconds to draw is shown in red
 A total of 2890 characters are drawn in each font
 
 Needs fonts 2, 4, 6, 7 and 8

 Make sure all the required fonts are loaded by editting the
 User_Setup.h file in the TFT_ILI9341 library folder.

 If using an UNO or Mega (ATmega328 or ATmega2560 processor) then for best
 performance use the F_AS_T option found in the User_Setup.h file in the
 TFT_ILI9341 library folder.

 The library uses the hardware SPI pins only:
   For UNO, Nano, Micro Pro ATmega328 based processors
      MOSI = pin 11, SCK = pin 13
   For Mega:
      MOSI = pin 51, SCK = pin 52

 The pins used for the TFT chip select (CS) and Data/command (DC) and Reset (RST)
 signal lines to the TFT must also be defined in the library User_Setup.h file.

 Sugested TFT connections for UNO and Atmega328 based boards
   sclk 13  // Don't change, this is the hardware SPI SCLK line
   mosi 11  // Don't change, this is the hardware SPI MOSI line
   cs   10  // Chip select for TFT display
   dc   9   // Data/command line
   rst  7   // Reset, you could connect this to the Arduino reset pin

 Suggested TFT connections for the MEGA and ATmega2560 based boards
   sclk 52  // Don't change, this is the hardware SPI SCLK line
   mosi 51  // Don't change, this is the hardware SPI MOSI line
   cs   47  // TFT chip select line
   dc   48  // TFT data/command line
   rst  44  // you could alternatively connect this to the Arduino reset

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  ######       TO SELECT THE FONTS AND PINS YOU USE, SEE ABOVE       ######
  #########################################################################
 */


#include <TFT_ILI9341.h> // Hardware-specific library
#include <SPI.h>
#define WAIT 0
TFT_ILI9341 tft = TFT_ILI9341();       // Invoke custom library

unsigned long drawTime = 0;
float ValueX=0;

const unsigned int X_RESET_BUTTON_PIN = 6;
const unsigned int Y_RESET_BUTTON_PIN = 15;
const unsigned int Z_RESET_BUTTON_PIN = 16;

volatile long xResetValue;
volatile long yResetValue;
volatile long zResetValue;

// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
boolean isNegativeX = false;
boolean isNegativeY = false;
boolean isNegativeZ = false;
boolean isNegativeA = false;

// DRO config (if axis is not connected change in the corresponding constant value from "1" to "0")
#define SCALE_X_ENABLED 1
#define SCALE_Y_ENABLED 1
#define SCALE_Z_ENABLED 1
#define SCALE_W_ENABLED 0

// I/O ports config (change pin numbers if DRO, Tach sensor or Tach LED feedback is connected to different ports)
#define SCALE_CLK_PIN 2

#define SCALE_X_PIN 3
#define SCALE_Y_PIN 4
#define SCALE_Z_PIN 5
#define SCALE_W_PIN 6

// DRO rounding On/Off (if not enabled change in the corresponding constant value from "1" to "0")
#define SCALE_X_AVERAGE_ENABLED 1
#define SCALE_Y_AVERAGE_ENABLED 1
#define SCALE_Z_AVERAGE_ENABLED 1
#define SCALE_W_AVERAGE_ENABLED 1

// DRO rounding sample size.  Change it to 16 for machines with power feed
#define AXIS_AVERAGE_COUNT 24

// General Settings
#define UART_BAUD_RATE 9600				//  Set this so it matches the BT module's BAUD rate 
#define UPDATE_FREQUENCY 24				//  Frequency in Hz (number of timer per second the scales are read and the data is sent to the application)
#define TACH_UPDATE_FREQUENCY 4 		//  Max Frequency in Hz (number of timer per second) the tach output is sent to the application

//---END OF CONFIGURATION PARAMETERS ---


//---DO NOT CHANGE THE CODE BELOW UNLESS YOU KNOW WHAT YOU ARE DOING ---

/* iGaging Clock Settings (do not change) */
#define SCALE_CLK_PULSES 21				//iGaging and Accuremote scales use 21 bit format
#define SCALE_CLK_FREQUENCY 9000		//iGaging scales run at about 9-10KHz
#define SCALE_CLK_DUTY 20				// iGaging scales clock run at 20% PWM duty (22us = ON out of 111us cycle)

/* weighted average constants */ 
#define FILTER_SLOW_EMA AXIS_AVERAGE_COUNT	// Slow movement EMA
#define FILTER_FAST_EMA 2 					// Fast movement EMA


#if (SCALE_X_ENABLED > 0) || (SCALE_Y_ENABLED > 0) || (SCALE_Z_ENABLED > 0) || (SCALE_W_ENABLED > 0)
#define DRO_ENABLED 1
#else
#define DRO_ENABLED 0
#endif

#if (SCALE_X_AVERAGE_ENABLED > 0) || (SCALE_Y_AVERAGE_ENABLED > 0) || (SCALE_Z_AVERAGE_ENABLED > 0) || (SCALE_W_AVERAGE_ENABLED > 0)
#define SCALE_AVERAGE_ENABLED 1
#else
#define SCALE_AVERAGE_ENABLED 0
#endif

// Define registers and pins for ports
#if SCALE_CLK_PIN < 8 
#define CLK_PIN_BIT SCALE_CLK_PIN
#define SCALE_CLK_DDR DDRD
#define SCALE_CLK_OUTPUT_PORT PORTD
#else
#define CLK_PIN_BIT (SCALE_CLK_PIN - 8)
#define SCALE_CLK_DDR DDRB
#define SCALE_CLK_OUTPUT_PORT PORTB
#endif

#if SCALE_X_PIN < 8 
#define X_PIN_BIT SCALE_X_PIN
#define X_DDR DDRD
#define X_INPUT_PORT PIND
#else
#define X_PIN_BIT (SCALE_X_PIN - 8)
#define X_DDR DDRB
#define X_INPUT_PORT PINB
#endif

#if SCALE_Y_PIN < 8 
#define Y_PIN_BIT SCALE_Y_PIN
#define Y_DDR DDRD
#define Y_INPUT_PORT PIND
#else
#define Y_PIN_BIT (SCALE_Y_PIN - 8)
#define Y_DDR DDRB
#define Y_INPUT_PORT PINB
#endif

#if SCALE_Z_PIN < 8 
#define Z_PIN_BIT SCALE_Z_PIN
#define Z_DDR DDRD
#define Z_INPUT_PORT PIND
#else
#define Z_PIN_BIT (SCALE_Z_PIN - 8)
#define Z_DDR DDRB
#define Z_INPUT_PORT PINB
#endif

#if SCALE_W_PIN < 8 
#define W_PIN_BIT SCALE_W_PIN
#define W_DDR DDRD
#define W_INPUT_PORT PIND
#else
#define W_PIN_BIT (SCALE_W_PIN - 8)
#define W_DDR DDRB
#define W_INPUT_PORT PINB
#endif

// Some constants calculated here

long const longMax = __LONG_MAX__;
long const longMin = (- __LONG_MAX__ - (long) 1);
long const slowSc = ((long) 2000) / (((long) FILTER_SLOW_EMA) + ((long) 1));
long const fastSc = ((long) 20) / (((long) FILTER_FAST_EMA) + ((long) 1));

int const updateFrequencyCounterLimit = (int) (((unsigned long) SCALE_CLK_FREQUENCY) /((unsigned long) UPDATE_FREQUENCY));
int const clockCounterLimit = (int) (((unsigned long) 2000000) / (unsigned long) SCALE_CLK_FREQUENCY) - 10;
int const scaleClockDutyLimit = (int) (((unsigned long) 20000) * ((unsigned long) SCALE_CLK_DUTY) / (unsigned long) SCALE_CLK_FREQUENCY);


//variables that will store the DRO readout
volatile boolean tickTimerFlag;
volatile int updateFrequencyCounter;

// Axis count
#if SCALE_X_ENABLED > 0
volatile long xValue;
volatile long xReportedValue;
volatile long xDisplayValue;
#endif
#if SCALE_X_AVERAGE_ENABLED > 0
volatile long axisLastReadX[AXIS_AVERAGE_COUNT];
volatile int axisLastReadPositionX;
volatile long axisAMAValueX;
#endif

#if SCALE_Y_ENABLED > 0
volatile long yValue;
volatile long yReportedValue;
#endif
#if SCALE_Y_AVERAGE_ENABLED > 0
volatile long axisLastReadY[AXIS_AVERAGE_COUNT];
volatile int axisLastReadPositionY;
volatile long axisAMAValueY;
#endif

#if SCALE_Z_ENABLED > 0
volatile long zValue;
volatile long zReportedValue;
#endif
#if SCALE_Z_AVERAGE_ENABLED > 0
volatile long axisLastReadZ[AXIS_AVERAGE_COUNT];
volatile int axisLastReadPositionZ;
volatile long axisAMAValueZ;
#endif

#if SCALE_W_ENABLED > 0
volatile long wValue;
volatile long wReportedValue;
#endif
#if SCALE_W_AVERAGE_ENABLED > 0
volatile long axisLastReadW[AXIS_AVERAGE_COUNT];
volatile int axisLastReadPositionW;
volatile long axisAMAValueW;
#endif

int   X_RESET_BUTTON_STATE;
int   Y_RESET_BUTTON_STATE;
int   Z_RESET_BUTTON_STATE;

//The setup function is called once at startup of the sketch
void setup()
{
  tft.init();
  tft.setRotation(3);
  
  pinMode(X_RESET_BUTTON_PIN, INPUT);
  pinMode(Y_RESET_BUTTON_PIN, INPUT);
  pinMode(Z_RESET_BUTTON_PIN, INPUT);

  xResetValue=0;
  yResetValue=0;
  zResetValue=0;

// Initialize DRO values
#if DRO_ENABLED > 0
	
	// clock pin should be set as output
	SCALE_CLK_DDR |= _BV(CLK_PIN_BIT);
	// set the clock pin to low
	SCALE_CLK_OUTPUT_PORT &= ~_BV(CLK_PIN_BIT);

	//data pins should be set as inputs
  #if SCALE_X_ENABLED > 0
	  X_DDR &= ~_BV(X_PIN_BIT);
	  xValue = 0L;
	  xReportedValue = 0L;
    #if SCALE_X_AVERAGE_ENABLED > 0
	    initializeAxisAverage(axisLastReadX, axisLastReadPositionX, axisAMAValueX);
    #endif
  #endif

  #if SCALE_Y_ENABLED > 0
	  Y_DDR &= ~_BV(Y_PIN_BIT);
	  yValue = 0L;
	  yReportedValue = 0L;
    #if SCALE_Y_AVERAGE_ENABLED > 0
	    initializeAxisAverage(axisLastReadY, axisLastReadPositionY, axisAMAValueY);
    #endif
  #endif
  
  #if SCALE_Z_ENABLED > 0
		Z_DDR &= ~_BV(Z_PIN_BIT);
	  zValue = 0L;
	  zReportedValue = 0L;
    #if SCALE_Z_AVERAGE_ENABLED > 0
	    initializeAxisAverage(axisLastReadZ, axisLastReadPositionZ, axisAMAValueZ);
    #endif
  #endif
  
  #if SCALE_W_ENABLED > 0
	  W_DDR &= ~_BV(W_PIN_BIT);
	  wValue = 0L;
	  wReportedValue = 0L;
    #if SCALE_W_AVERAGE_ENABLED > 0
	    initializeAxisAverage(axisLastReadW, axisLastReadPositionW, axisAMAValueW);
    #endif
  #endif
#endif

	//initialize serial port
	Serial.begin(UART_BAUD_RATE);

	//initialize timers
	setupClkTimer();

	sei();	
  
  xResetValue=0;
  yResetValue=0;
  zResetValue=0;
  X_RESET_BUTTON_STATE=LOW;
  Y_RESET_BUTTON_STATE=LOW;
  Z_RESET_BUTTON_STATE=LOW;
  
tft.fillScreen(TFT_BLACK);
}


// The loop function is called in an endless loop
void loop()
{


Y_RESET_BUTTON_STATE = digitalRead(Y_RESET_BUTTON_PIN);
Z_RESET_BUTTON_STATE = digitalRead(Z_RESET_BUTTON_PIN);

 /*
    drawTime = millis();
    tft.drawString(":", 55, 80, 7);
  for (int i = 0; i < 1000; i++) {

    tft.drawNumber(i, 80, 80, 7);
  }

  drawTime = millis() - drawTime;
*/
    int posX=50;
    int posY=25;
    int IntegerPart=0;
	if (tickTimerFlag) 
	{

  	tickTimerFlag = false;
    #if DRO_ENABLED > 0
    		//print DRO positions to the serial port
      #if SCALE_X_ENABLED > 0
        #if SCALE_X_AVERAGE_ENABLED > 0
        		scaleValueRounded(xReportedValue, axisLastReadX, axisLastReadPositionX, axisAMAValueX);
           X_RESET_BUTTON_STATE = digitalRead(X_RESET_BUTTON_PIN);

            if (X_RESET_BUTTON_STATE == HIGH) 
            {
              Serial.println(F("HIGH"));
              X_RESET_BUTTON_STATE=LOW;
              xResetValue=xReportedValue;
               X_RESET_BUTTON_STATE = digitalRead(X_RESET_BUTTON_PIN);
              Serial.println((X_RESET_BUTTON_STATE));
            } 
            else
            {
             if(xResetValue<0)
              xDisplayValue=xReportedValue+abs(xResetValue);
              else
              xDisplayValue=xReportedValue-abs(xResetValue);
          
              float xValue=xDisplayValue;
              if(xValue<0)
              {
              isNegativeX=true;
              xValue=abs(xValue);
              }
              else
              {
              isNegativeX=false;  
              }
                
                tft.setTextColor(TFT_RED, TFT_BLACK);
                tft.setCursor(20, 40);
                tft.setTextSize(4);
                tft.print("X");
                tft.setCursor(260, 47);
                tft.setTextSize(3);
                tft.print("mm");
              
                tft.setCursor(20, 100);
                tft.setTextSize(4);
                tft.print("Y");
                tft.setCursor(260, 107);
                tft.setTextSize(3);
                tft.print("mm");
              
                tft.setCursor(20, 160);
                tft.setTextSize(4);
                tft.print("Z");
                tft.setCursor(260, 167);
                tft.setTextSize(3);
                tft.print("mm");
                
              
               //Draw X values 
                tft.setTextSize(1);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                 posX=50;
                 posY=25;
                if(isNegativeX==false)
                  tft.drawString("  ", posX,posY, 6);
                else
                  tft.drawString("-", posX, posY, 6);
                
                xValue=xValue/100;
              
                 IntegerPart=abs((int)xValue);
                posX=80;
                posY=20;
                if(IntegerPart<100)
                {
                  tft.setTextColor(TFT_BLACK, TFT_BLACK);
                   tft.drawNumber(0,  posX, posY, 7);
                   tft.setTextColor(TFT_WHITE, TFT_BLACK);
                   posX=posX+30;
                }
                  tft.drawFloat(xValue,2, posX, posY, 7);
    


            }
              
        #endif
        /*
      		Serial.print(F("R:"));
      		Serial.print((long)xResetValue);
          Serial.print(F("- V:"));
          Serial.print((long)xReportedValue);
      		Serial.print(F(";"));
         */
      #endif
    
      #if SCALE_Y_ENABLED > 0
        #if SCALE_Y_AVERAGE_ENABLED > 0
        		scaleValueRounded(yReportedValue, axisLastReadY, axisLastReadPositionY, axisAMAValueY);
        #endif
        /*
      		Serial.print(F("Y"));
      		Serial.print((long)yResetValue);
      		Serial.print(F(";"));
         */
      #endif
    
      #if SCALE_Z_ENABLED > 0
        #if SCALE_Z_AVERAGE_ENABLED > 0
        		scaleValueRounded(zReportedValue, axisLastReadZ, axisLastReadPositionZ, axisAMAValueZ);
        #endif
        /*
      		Serial.print(F("Z"));
      		Serial.print((long)zResetValue);
      		Serial.print(F(";"));
         */
      #endif
    
      #if SCALE_W_ENABLED > 0
        #if SCALE_W_AVERAGE_ENABLED > 0
        		scaleValueRounded(wReportedValue, axisLastReadW, axisLastReadPositionW, axisAMAValueW);
        #endif
        /*
      		Serial.print(F("W"));
      		Serial.print((long)wReportedValue);
      		Serial.print(F(";"));
         */
      #endif
    #endif
        // Serial.println("");   
  	}


    float yValue=yReportedValue;
    if(yValue<0)
    {
    isNegativeY=true;
    yValue=abs(yValue);
    }
    else
    {
    isNegativeY=false;  
    }

    float zValue=zReportedValue;
    if(zValue<0)
    {
    isNegativeZ=true;
    zValue=abs(zValue);
    }
    else
    {
    isNegativeZ=false;  
    }
 
 //Draw Y values 
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  posX=50;
  posY=85;
  if(isNegativeY==false)
    tft.drawString("  ", posX,posY, 6);
  else
    tft.drawString("-", posX, posY, 6);
  
  yValue=yValue/100;

  IntegerPart=abs((int)yValue);
  posX=80;
  posY=80;
  if(IntegerPart<100)
  {
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
     tft.drawNumber(0,  posX, posY, 7);
     tft.setTextColor(TFT_WHITE, TFT_BLACK);
     posX=posX+30;
  }
    tft.drawFloat(yValue,2, posX, posY, 7);

 //Draw Z values 
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  posX=50;
  posY=145;
  if(isNegativeZ==false)
    tft.drawString("  ", posX,posY, 6);
  else
    tft.drawString("-", posX, posY, 6);
  
  zValue=zValue/100;

  IntegerPart=abs((int)zValue);
  posX=80;
  posY=140;
  if(IntegerPart<100)
  {
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
     tft.drawNumber(0,  posX, posY, 7);
     tft.setTextColor(TFT_WHITE, TFT_BLACK);
     posX=posX+30;
  }
    tft.drawFloat(zValue,2, posX, posY, 7);

}


//initializes clock timer
void setupClkTimer()
{
	updateFrequencyCounter = 0;

	TCCR2A = 0;			// set entire TCCR2A register to 0
	TCCR2B = 0;			// same for TCCR2B

	// set compare match registers
#if DRO_ENABLED > 0
	OCR2A = scaleClockDutyLimit;			// default 44 = 22us
#else
	OCR2A = clockCounterLimit - 1;
#endif
	OCR2B = clockCounterLimit;			// default 222 = 111us

	// turn on Fast PWM mode
	TCCR2A |= _BV(WGM21) | _BV(WGM20);

	// Set CS21 bit for 8 prescaler //CS20 for no prescaler
	TCCR2B |= _BV(CS21);

	//initialize counter value to start at low pulse
#if DRO_ENABLED > 0
	TCNT2  = scaleClockDutyLimit + 1;
#else
	TCNT2  = 0;
#endif
	// enable timer compare interrupt A and B
	TIMSK2 |= _BV(OCIE2A) | _BV(OCIE2B);
	
}



/* Interrupt Service Routines */

// Timer 2 interrupt B ( Switches clock pin from low to high 21 times) at the end of clock counter limit
ISR(TIMER2_COMPB_vect) {

	// Set counter back to zero  
	TCNT2  = 0;  
#if DRO_ENABLED > 0
	// Only set the clock high if updateFrequencyCounter less than 21
	if (updateFrequencyCounter < SCALE_CLK_PULSES) {
		// Set clock pin high
		SCALE_CLK_OUTPUT_PORT |= _BV(CLK_PIN_BIT);
	}
#endif
}


// Timer 2 interrupt A ( Switches clock pin from high to low) at the end of clock PWM Duty counter limit
ISR(TIMER2_COMPA_vect) 
{
#if DRO_ENABLED > 0

	// Contorl the scale clock for only first 21 loops
	if (updateFrequencyCounter < SCALE_CLK_PULSES) {
	
		// Set clock low if high and then delay 2us
		if (SCALE_CLK_OUTPUT_PORT & _BV(CLK_PIN_BIT)) {
			SCALE_CLK_OUTPUT_PORT &= ~_BV(CLK_PIN_BIT);
			TCNT2  = scaleClockDutyLimit - 4; 
			return;
		}

		// read the pin state and shift it into the appropriate variables
		// Logic by Les Jones:
		//	If data pin is HIGH set bit 20th of the axis value to '1'.  Then shift axis value one bit to the right
		//  This is called 20 times (for bits received from 0 to 19)
		if (updateFrequencyCounter < SCALE_CLK_PULSES - 1) {
#if SCALE_X_ENABLED > 0
			if (X_INPUT_PORT & _BV(X_PIN_BIT))
				xValue |= ((long)0x00100000 );
			xValue >>= 1;
#endif

#if SCALE_Y_ENABLED > 0
			if (Y_INPUT_PORT & _BV(Y_PIN_BIT))
				yValue |= ((long)0x00100000 );
			yValue >>= 1;
#endif

#if SCALE_Z_ENABLED > 0
			if (Z_INPUT_PORT & _BV(Z_PIN_BIT))
				zValue |= ((long)0x00100000 );
			zValue >>= 1;
#endif
/*
#if SCALE_W_ENABLED > 0
			if (W_INPUT_PORT & _BV(W_PIN_BIT))
				wValue |= ((long)0x00100000 );
			wValue >>= 1;
#endif
*/

		} else if (updateFrequencyCounter == SCALE_CLK_PULSES - 1) {

			//If 21-st bit is 'HIGH' inverse the sign of the axis readout
#if SCALE_X_ENABLED > 0
			if (X_INPUT_PORT & _BV(X_PIN_BIT))
				xValue |= ((long)0xfff00000);
			xReportedValue = xValue;
			xValue = 0L;
#endif

#if SCALE_Y_ENABLED > 0
			if (Y_INPUT_PORT & _BV(Y_PIN_BIT))
				yValue |= ((long)0xfff00000);
			yReportedValue = yValue;
			yValue = 0L;
#endif

#if SCALE_Z_ENABLED > 0
			if (Z_INPUT_PORT & _BV(Z_PIN_BIT))
				zValue |= ((long)0xfff00000);
			zReportedValue = zValue;
			zValue = 0L;
#endif
/*
#if SCALE_W_ENABLED > 0
			if (W_INPUT_PORT & _BV(W_PIN_BIT))
				wValue |= ((long)0xfff00000);
			wReportedValue = wValue;
			wValue = 0L;
#endif
*/
			// Tell the main loop, that it's time to sent data
			tickTimerFlag = true;

		}
	}
#else
	if (updateFrequencyCounter == 0) {
		// Tell the main loop, that it's time to sent data
		tickTimerFlag = true;
	}
#endif
	
	updateFrequencyCounter++;
	// Start of next cycle 
	if ( updateFrequencyCounter >= updateFrequencyCounterLimit) {
		updateFrequencyCounter = 0;
	}

}


#if DRO_ENABLED > 0
#if SCALE_AVERAGE_ENABLED > 0
inline void	initializeAxisAverage(volatile long axisLastRead[], volatile int &axisLastReadPosition, volatile long &axisAMAValue) {
	
	for (axisLastReadPosition = 0; axisLastReadPosition < (int) AXIS_AVERAGE_COUNT; axisLastReadPosition++) {
		axisLastRead[axisLastReadPosition] = 0;
	}
	axisLastReadPosition = 0;
	axisAMAValue = 0;

}

inline void scaleValueRounded(volatile long &ReportedValue, volatile long axisLastRead[], volatile int &axisLastReadPosition, volatile long &axisAMAValue)
{

	int last_pos; 
	int first_pos;
	int prev_pos;
	int filter_pos;


	long dir;
	long minValue = longMax;
	long maxValue = longMin;
	long volatility = 0;
	long valueRange;
	long ssc;
	long constant;
	long delta;

	// Save current read and increment position 
	axisLastRead[axisLastReadPosition] = ReportedValue;
	last_pos = axisLastReadPosition;

	axisLastReadPosition++;
	if (axisLastReadPosition == (int) AXIS_AVERAGE_COUNT) {
		axisLastReadPosition = 0;
	}
	first_pos = axisLastReadPosition;
	
    dir = (axisLastRead[first_pos] - axisLastRead[last_pos]) * ((long) 100);

    // Calculate the volatility in the counts by taking the sum of the differences
    prev_pos = first_pos;
    for (filter_pos = (first_pos + 1) % AXIS_AVERAGE_COUNT;
         filter_pos != first_pos;
         filter_pos = (filter_pos + 1) % AXIS_AVERAGE_COUNT)
    {
        minValue = MIN(minValue, axisLastRead[filter_pos]);
        maxValue = MAX(maxValue, axisLastRead[filter_pos]);
        volatility += ABS(axisLastRead[filter_pos] - axisLastRead[prev_pos]);
        prev_pos = filter_pos;
    }

    // Just return the read if there is no volatility to avoid divide by 0
    if (volatility == (long) 0)
    {
		axisAMAValue = axisLastRead[last_pos] * ((long) 100);
		return;
    }
	
    // If the last AMA is not within twice the sample range, then assume the position jumped
    // and reset the AMA to the current read
	maxValue = maxValue * ((long) 100);
	minValue = minValue * ((long) 100);
    valueRange = maxValue - minValue;
    if (axisAMAValue > maxValue + valueRange + ((long) 100) ||
        axisAMAValue < minValue - valueRange - ((long) 100))
    {
		axisAMAValue = axisLastRead[last_pos] * ((long) 100);
		return;
    }

    // Calculate the smoothing constant
    ssc = (ABS(dir / volatility) * fastSc) + slowSc;
    constant = (ssc * ssc) / ((long) 10000);

    // Calculate the new average
	delta = axisLastRead[last_pos] - (axisAMAValue / ((long) 100));
	axisAMAValue = axisAMAValue + constant * delta; 

    ReportedValue = (axisAMAValue + ((long) 50)) / ((long) 100);
	return;

}

inline long MIN(long value1, long value2){
	if(value1 > value2) {
		return value2;
	} else {
		return value1;
	}
}

inline long MAX(long value1, long value2){
	if(value1 > value2) {
		return value1;
	} else {
		return value2;
	}
}

inline long ABS(long value){
	if(value < 0) {
		return -value;
	} else {
		return value;
	}
}

#endif
#endif

