/*******************************************************************************
* Title: Reflow Oven Controller - Slave 1 or Slave 2 - Program
* Version: 1.0
* Date: 09/04/2016
* Company: DruTech Corp
* Author: Ovidiu Carstea
* Website: www.ovidiucarstea.com
*==========================================
*To Do: 
*Read reflow parameters from SD Card updated from Master Program
*Use Master with touch screen
*Connect Buzzer
*==========================================
* Credentials
* ==========================================
* Initial Author: Lim Phang Moh
* Website: www.rocketscream.com
* ==========================================
* Brett Beauregard (www.brettbeauregard.com)
* ==========================================
* Author of Arduino PID library. On top of providing industry standard PID 
* implementation, he gave a lot of help in making this reflow oven controller 
* possible using his awesome library.
* ==========================================
* Reflow oven controler
* ==========================================
* Controls 2 Control Relays MCPC1225 for 2 heating elements
* Control1 1 Relay for Exhoust Fan
* There are 3 different Arduino Uno R3, one for each relay control
* One Arduino Uno will be master and 2 will be slave
* Arduino Master:
* -Control 2 Arduino slaves
* -Control the Fan Relay
* -Display Status on LCD
* -Control SW3 switch for every Arduino Slave
* -Control SW2 switch for every MCPC1225 relay
* Arduino Slave 1 and 2
* -Each arduino control one MCPC1225 relay and read 
*  temperature from a Thermocouple with Thermo Click
* ==========================================
* This program is for Arduino Slave 1 or Slave 2
* ==========================================
* Required hardware:
* 3 x Arduino Uno R3
* 
* 2 x MCPC1225 Relay Controler
* >>http://www.mouser.com/ProductDetail/Crydom/MCPC1225A
* 
* 2 x Thermo Click
* >>http://www.mikroe.com/click/thermo/
* 
* 2 x Thermocouple Type-K Glass Braid Insulated
* >>http://www.mikroe.com/click/thermo/
* 
*1 x LED
*1 x Main Power Switch - SW1
*1 x Switch Puss ON/ Push OFF - SW2
*1 x Switch Temporary Push - SW3
*1 x 1.0 MicroF unpolarized capacitor
*1 x 1k Rezistor
*1 x 4k7 Rezistor
*1 x 10k Rezistor
*1 x Power Supply 8-12V
*
*
* Temperature (Degree Celcius)                 Peek Temperature
* 245-|                                               x  x  
*     |                                            x        x
*     |                                         x              x
*     |                                      x                    x
* 200-|                                   x                          x
*     |                              x    |                          |   x   
*     |                         x         |                          |       x
*     |                    x              |                          |
* 150-|               x                   |                          |
*     |             x |                   |                          |
*     |           x   |                   |                          | 
*     |         x     |                   |                          | 
*     |       x       |                   |                          | 
*     |     x         |                   |                          |
*     |   x           |                   |                          |
* 30 -| x             |                   |                          |
*     |<  60 - 90 s  >|<    90 - 120 s   >|<       90 - 120 s       >|
*     | Preheat Stage |   Soaking Stage   |       Reflow Stage       | Cool
*  0  |_ _ _ _ _ _ _ _|_ _ _ _ _ _ _ _ _ _|_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 
*                                                                Time (Seconds)
*

* 
*
* Required Libraries
* ==================
* - Arduino PID Library: 
*   >> https://github.com/br3ttb/Arduino-PID-Library
* - MAX31855 Library (for board v1.60 & above): 
*   >> https://github.com/rocketscream/MAX31855
*******************************************************************************/
#define  USE_MAX31855
// ***** INCLUDES *****
#include <LiquidCrystal.h>
#include <MAX31855.h>
#include <PID_v1.h>

// ***** TYPE DEFINITIONS *****
typedef enum REFLOW_STATE
{
  REFLOW_STATE_IDLE,
  REFLOW_STATE_PREHEAT,
  REFLOW_STATE_SOAK,
  REFLOW_STATE_REFLOW,
  REFLOW_STATE_COOL,
  REFLOW_STATE_COMPLETE,
	REFLOW_STATE_TOO_HOT,
  REFLOW_STATE_ERROR
} reflowState_t;

typedef enum REFLOW_STATUS
{
  REFLOW_STATUS_OFF,
  REFLOW_STATUS_ON
} reflowStatus_t;

typedef	enum SWITCH
{
	SWITCH_NONE,
	SWITCH_1,	
	SWITCH_2
}	switch_t;

typedef enum DEBOUNCE_STATE
{
  DEBOUNCE_STATE_IDLE,
  DEBOUNCE_STATE_CHECK,
  DEBOUNCE_STATE_RELEASE
} debounceState_t;

// ***** CONSTANTS *****
#define TEMPERATURE_ROOM 50
#define TEMPERATURE_SOAK_MIN 150
#define TEMPERATURE_SOAK_MAX 200
#define TEMPERATURE_REFLOW_MAX 250
#define TEMPERATURE_COOL_MIN 100
#define SENSOR_SAMPLING_TIME 1000
#define SOAK_TEMPERATURE_STEP 5
#define SOAK_MICRO_PERIOD 9000
#define DEBOUNCE_PERIOD_MIN 50

// ***** PID PARAMETERS *****
// ***** PRE-HEAT STAGE *****
#define PID_KP_PREHEAT 100
#define PID_KI_PREHEAT 0.025
#define PID_KD_PREHEAT 20
// ***** SOAKING STAGE *****
#define PID_KP_SOAK 300
#define PID_KI_SOAK 0.05
#define PID_KD_SOAK 250
// ***** REFLOW STAGE *****
#define PID_KP_REFLOW 300
#define PID_KI_REFLOW 0.05
#define PID_KD_REFLOW 350
#define PID_SAMPLE_TIME 1000

// ***** LCD MESSAGES *****
const char* lcdMessagesReflowStatus[] = {
  "Ready",
  "Pre-heat",
  "Soak",
  "Reflow",
  "Cool",
  "Complete",
	"Wait,hot",
  "Error"
};

// ***** DEGREE SYMBOL FOR LCD *****
unsigned char degree[8]  = {
  140,146,146,140,128,128,128,128};

// ***** PIN ASSIGNMENT *****
	int ssrPin = 5;              //D5 - DC output -> 
	int thermocoupleSOPin = A3;  //SDO - Thermo CLick
	int thermocoupleCSPin = A2;  //CS  - Thermo CLick
	int thermocoupleCLKPin = A1; //CSR - Thermo CLick
	int lcdRsPin = 7;
	int lcdEPin = 8;
	int lcdD4Pin = 9;
	int lcdD5Pin = 10;
	int lcdD6Pin = 11;
	int lcdD7Pin = 12;
	int ledRedPin = 4;            //D4 - LED ON/OFF indicator (optional)
	int buzzerPin = 6;
	int switchPin = A0;           //A0 - Switch SW3 Temporary - Push once to start the cicle


// ***** PID CONTROL VARIABLES *****
double ssrPinAnalogValue=0;
double outputAnalogValue=0;

double setpoint;
double input;
double output;
double kp = PID_KP_PREHEAT;
double ki = PID_KI_PREHEAT;
double kd = PID_KD_PREHEAT;
int windowSize;
unsigned long windowStartTime;
unsigned long nextCheck;
unsigned long nextRead;
unsigned long timerSoak;
unsigned long buzzerPeriod;
// Reflow oven controller state machine state variable
reflowState_t reflowState;
// Reflow oven controller status
reflowStatus_t reflowStatus;
// Switch debounce state machine state variable
debounceState_t debounceState;
// Switch debounce timer
long lastDebounceTime;
// Switch press status
switch_t switchStatus;
// Seconds timer
int timerSeconds;

// Specify PID control interface
PID reflowOvenPID(&input, &output, &setpoint, kp, ki, kd, DIRECT);
// Specify LCD interface
LiquidCrystal lcd(lcdRsPin, lcdEPin, lcdD4Pin, lcdD5Pin, lcdD6Pin, lcdD7Pin);

MAX31855 thermocouple(thermocoupleSOPin, thermocoupleCSPin,thermocoupleCLKPin);


void setup()
{
    // Serial communication at 57600 bps
  Serial.begin(57600);
  Serial.print("\n");
  Serial.println("Reflow Oven");
  Serial.println("PID Controller");
  
  // SSR pin initialization to ensure reflow oven is off
  //digitalWrite(ssrPin, LOW);
  analogWrite(ssrPin, 0);
  
  pinMode(ssrPin, OUTPUT);

  // Buzzer pin initialization to ensure annoying buzzer is off
  digitalWrite(buzzerPin, LOW);
  pinMode(buzzerPin, OUTPUT);

  // LED pins initialization and turn on upon start-up (active low)
  digitalWrite(ledRedPin, LOW);
  pinMode(ledRedPin, OUTPUT);
  // Start-up splash
  digitalWrite(buzzerPin, HIGH);
  lcd.begin(8, 2);
  lcd.createChar(0, degree);
  lcd.clear();
  lcd.print("Reflow");
  lcd.setCursor(0, 1);
  lcd.print("Oven 1.2");
  digitalWrite(buzzerPin, LOW);
  delay(2500);
  lcd.clear();

  // Serial communication at 57600 bps
  Serial.begin(57600);

  // Turn off LED (active low)
  digitalWrite(ledRedPin, HIGH);

  // Set window size
  windowSize = 2000;
  // Initialize time keeping variable
  nextCheck = millis();
  // Initialize thermocouple reading variable
  nextRead = millis();
}

void loop()
{
  // Current time
  unsigned long now;

  // Time to read thermocouple?
  if (millis() > nextRead)
  {
    // Read thermocouple next sampling period
    nextRead += SENSOR_SAMPLING_TIME;
    // Read current temperature
		input = thermocouple.readThermocouple(CELSIUS);

		
    // If thermocouple problem detected
			if((input == FAULT_OPEN) || (input == FAULT_SHORT_GND) || (input == FAULT_SHORT_VCC))
		{
      // Illegal operation
      reflowState = REFLOW_STATE_ERROR;
      reflowStatus = REFLOW_STATUS_OFF;
    }
  }

  if (millis() > nextCheck)
  {
    // Check input in the next seconds
    nextCheck += 1000;
    // If reflow process is on going
    if (reflowStatus == REFLOW_STATUS_ON)
    {
      // Toggle red LED as system heart beat
      digitalWrite(ledRedPin, !(digitalRead(ledRedPin)));
      // Increase seconds timer for reflow curve analysis
      timerSeconds++;
      // Send temperature and time stamp to serial 
      
      Serial.print(timerSeconds);
      Serial.print(" ");
      Serial.print(setpoint);
      Serial.print(" ");
      Serial.print(input);
      Serial.print(" ");
      Serial.print(lcdMessagesReflowStatus[reflowState]);
      outputAnalogValue=output/2000*255;
      Serial.print(" ");
      Serial.print(String(outputAnalogValue/255*5));
      Serial.print("\n");
    }
    else
    {
      // Turn off red LED
      digitalWrite(ledRedPin, HIGH);
    }

    // Clear LCD
    lcd.clear();
    // Print current system state
    lcd.print(lcdMessagesReflowStatus[reflowState]);
    // Move the cursor to the 2 line
    lcd.setCursor(0, 1);

    // If currently in error state
    if (reflowState == REFLOW_STATE_ERROR)
    {
      // No thermocouple wire connected
      lcd.print("TC Error!");
    }
    else
    {
      // Print current temperature
      lcd.print(input);

			#if ARDUINO >= 100
				// Print degree Celsius symbol
				lcd.write((uint8_t)0);
			#else
				// Print degree Celsius symbol
				lcd.print(0, BYTE);
			#endif
      lcd.print("C ");
    }
  }

  // Reflow oven controller state machine
  switch (reflowState)
  {
  case REFLOW_STATE_IDLE:
		// If oven temperature is still above room temperature
		if (input >= TEMPERATURE_ROOM)
		{
			reflowState = REFLOW_STATE_TOO_HOT;
		}
		else
		{
			// If switch is pressed to start reflow process
			if (switchStatus == SWITCH_1)
			{
        // Send header for CSV file
        Serial.println("Time Setpoint Input Output");
        // Intialize seconds timer for serial debug information
        timerSeconds = 0;
        // Initialize PID control window starting time
        windowStartTime = millis();
        // Ramp up to minimum soaking temperature
        setpoint = TEMPERATURE_SOAK_MIN;
        // Tell the PID to range between 0 and the full window size
        reflowOvenPID.SetOutputLimits(0, windowSize);
        reflowOvenPID.SetSampleTime(PID_SAMPLE_TIME);
        // Turn the PID on
        reflowOvenPID.SetMode(AUTOMATIC);
        // Proceed to preheat stage
        reflowState = REFLOW_STATE_PREHEAT;
      }
    }
    break;

  case REFLOW_STATE_PREHEAT:
    reflowStatus = REFLOW_STATUS_ON;
    // If minimum soak temperature is achieve       
    if (input >= TEMPERATURE_SOAK_MIN)
    {
      // Chop soaking period into smaller sub-period
      timerSoak = millis() + SOAK_MICRO_PERIOD;
      // Set less agressive PID parameters for soaking ramp
      reflowOvenPID.SetTunings(PID_KP_SOAK, PID_KI_SOAK, PID_KD_SOAK);
      // Ramp up to first section of soaking temperature
      setpoint = TEMPERATURE_SOAK_MIN + SOAK_TEMPERATURE_STEP;   
      // Proceed to soaking state
      reflowState = REFLOW_STATE_SOAK; 
    }
    break;

  case REFLOW_STATE_SOAK:     
    // If micro soak temperature is achieved       
    if (millis() > timerSoak)
    {
      timerSoak = millis() + SOAK_MICRO_PERIOD;
      // Increment micro setpoint
      setpoint += SOAK_TEMPERATURE_STEP;
      if (setpoint > TEMPERATURE_SOAK_MAX)
      {
        // Set agressive PID parameters for reflow ramp
        reflowOvenPID.SetTunings(PID_KP_REFLOW, PID_KI_REFLOW, PID_KD_REFLOW);
        // Ramp up to first section of soaking temperature
        setpoint = TEMPERATURE_REFLOW_MAX;   
        // Proceed to reflowing state
        reflowState = REFLOW_STATE_REFLOW; 
      }
    }
    break; 

  case REFLOW_STATE_REFLOW:
    // We need to avoid hovering at peak temperature for too long
    // Crude method that works like a charm and safe for the components
    if (input >= (TEMPERATURE_REFLOW_MAX - 5))
    {
      // Set PID parameters for cooling ramp
      reflowOvenPID.SetTunings(PID_KP_REFLOW, PID_KI_REFLOW, PID_KD_REFLOW);
      // Ramp down to minimum cooling temperature
      setpoint = TEMPERATURE_COOL_MIN;   
      // Proceed to cooling state
      reflowState = REFLOW_STATE_COOL; 
    }
    break;   

  case REFLOW_STATE_COOL:
    // If minimum cool temperature is achieve       
    if (input <= TEMPERATURE_COOL_MIN)
    {
      // Retrieve current time for buzzer usage
      buzzerPeriod = millis() + 1000;
      // Turn on buzzer and green LED to indicate completion
			digitalWrite(buzzerPin, HIGH);
      // Turn off reflow process
      reflowStatus = REFLOW_STATUS_OFF;                
      // Proceed to reflow Completion state
      reflowState = REFLOW_STATE_COMPLETE; 
    }         
    break;    

  case REFLOW_STATE_COMPLETE:
    if (millis() > buzzerPeriod)
    {
      // Turn off buzzer and green LED
      digitalWrite(buzzerPin, LOW);
			// Reflow process ended
      reflowState = REFLOW_STATE_IDLE; 
    }
    break;
	
	case REFLOW_STATE_TOO_HOT:
		// If oven temperature drops below room temperature
		if (input < TEMPERATURE_ROOM)
		{
			// Ready to reflow
			reflowState = REFLOW_STATE_IDLE;
		}
		break;
		
  case REFLOW_STATE_ERROR:
    // If thermocouple problem is still present
		if((input == FAULT_OPEN) || (input == FAULT_SHORT_GND) || (input == FAULT_SHORT_VCC))
    {
      // Wait until thermocouple wire is connected
      reflowState = REFLOW_STATE_ERROR; 
    }
    else
    {
      // Clear to perform reflow process
      reflowState = REFLOW_STATE_IDLE; 
    }
    break;    
  }    

  // If switch 1 is pressed
  if (switchStatus == SWITCH_1)
  {
    // If currently reflow process is on going
    if (reflowStatus == REFLOW_STATUS_ON)
    {
      // Button press is for cancelling
      // Turn off reflow process
      reflowStatus = REFLOW_STATUS_OFF;
      // Reinitialize state machine
      reflowState = REFLOW_STATE_IDLE;
    }
  } 

  // Simple switch debounce state machine (for switch #1 (both analog & digital
	// switch supported))
  switch (debounceState)
  {
  case DEBOUNCE_STATE_IDLE:
    // No valid switch press
    switchStatus = SWITCH_NONE;
    // If switch #1 is pressed
			if (analogRead(switchPin) == 0)
			{
				// Intialize debounce counter
				lastDebounceTime = millis();
				// Proceed to check validity of button press
				debounceState = DEBOUNCE_STATE_CHECK;
			}	
    break;

  case DEBOUNCE_STATE_CHECK:
			// If switch #1 is still pressed
			if (analogRead(switchPin) == 0)
			{
				// If minimum debounce period is completed
				if ((millis() - lastDebounceTime) > DEBOUNCE_PERIOD_MIN)
				{
					// Proceed to wait for button release
					debounceState = DEBOUNCE_STATE_RELEASE;
				}
			}
			// False trigger
			else
			{
				// Reinitialize button debounce state machine
				debounceState = DEBOUNCE_STATE_IDLE; 
			}
    break;

  case DEBOUNCE_STATE_RELEASE:
			if (analogRead(switchPin) > 0)
		{
      // Valid switch 1 press
      switchStatus = SWITCH_1;
      // Reinitialize button debounce state machine
      debounceState = DEBOUNCE_STATE_IDLE; 
    }
    break;
  }

  // PID computation and SSR control
  if (reflowStatus == REFLOW_STATUS_ON)
  {

   /* outputAnalogValue=output/2000*255;
    if(outputAnalogValue!=ssrPinAnalogValue)
   {
    Serial.println("ssrPin: "+String(outputAnalogValue,2));
    ssrPinAnalogValue=outputAnalogValue;
   }
   */
    
    now = millis();

    reflowOvenPID.Compute();

    if((now - windowStartTime) > windowSize)
    { 
      // Time to shift the Relay Window
      windowStartTime += windowSize;
    }
    if(output > (now - windowStartTime)) 
    analogWrite(ssrPin, outputAnalogValue);
    else 
    analogWrite(ssrPin, 0);
      
  }
  else // Reflow oven process is off, ensure oven is off
  {
    analogWrite(ssrPin, 0);
  }
}
