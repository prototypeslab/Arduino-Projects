// EasyDriver connections
#define step_pin 9  // Pin 9 connected to Steps pin on EasyDriver
#define dir_pin 8   // Pin 8 connected to Direction pin
#define MS1 10       // Pin 10 connected to MS1 pin
#define MS2 11      // Pin 11 connected to MS2 pin
#define SLEEP 12     // Pin 12 connected to SLEEP pin
                    
volatile boolean TurnDetected;  // need volatile for Interrupts
volatile boolean rotationdirection;  // CW or CCW rotation

// Rotary Encoder Module connections
const int PinCLK=2;   // Generating interrupts using CLK signal
const int PinDT=3;    // Reading DT signal
const int PinSW=4;    // Reading Push Button switch

int StepperPosition=0;    // To store Stepper Motor Position
int StepsToTake=3;      // Controls the speed of the Stepper per Rotary click
 int counter = 0; 
int direction;   // Variable to set Rotation (CW-CCW) of stepper
int flag=0;

// Interrupt routine runs if CLK goes from HIGH to LOW
void rotarydetect ()  {
delayMicroseconds(400); // delay for Debouncing
if (digitalRead(PinCLK))
{
rotationdirection= digitalRead(PinDT);

}
else
{
rotationdirection= !digitalRead(PinDT);
}
TurnDetected = true;
if(rotationdirection)
         counter++;
         else
         counter--;
         
         Serial.print("Counter: ");
         Serial.println(counter);
}


void setup ()  {

   pinMode(MS1, OUTPUT);
   pinMode(MS2, OUTPUT);
   pinMode(dir_pin, OUTPUT);
   pinMode(step_pin, OUTPUT);
   pinMode(SLEEP, OUTPUT);   
   digitalWrite(SLEEP, HIGH);  // Wake up EasyDriver
   delay(5);  // Wait for EasyDriver wake up


   Serial.begin (9600);
 /* Configure type of Steps on EasyDriver:
 // MS1 MS2
 //
 // LOW LOW = Full Step //
 // HIGH LOW = Half Step //
 // LOW HIGH = A quarter of Step //
 // HIGH HIGH = An eighth of Step //
 */ 
   digitalWrite(MS1, LOW);      // Configures to Full Steps
   digitalWrite(MS2, LOW);    // Configures to Full Steps
   
  pinMode(PinCLK,INPUT);  // Set Pin to Input
  pinMode(PinDT,INPUT);  
  pinMode(PinSW,INPUT);
  digitalWrite(PinSW, HIGH); // Pull-Up resistor for switch
  attachInterrupt (0,rotarydetect,FALLING); // interrupt 0 always connected to pin 2 on Arduino UNO
  Serial.println("*** End Setup ***");
}


void loop ()  {
if (flag==0)
for (int x = 0; x < 200; x++) 
{
                digitalWrite(step_pin, HIGH);
              delayMicroseconds(700);
              digitalWrite(step_pin, LOW);
              delayMicroseconds(700);
  flag=1;
  }
/*
  if (!(digitalRead(PinSW))) 
  {   // check if button is pressed
  if (StepperPosition == 0) 
  {  // check if button was already pressed
    
  } 
  else 
  {
      if (StepperPosition > 0) 
      {  // Stepper was moved CW
        while (StepperPosition != 0)
        {  //  Do until Motor position is back to ZERO
          digitalWrite(dir_pin, HIGH);  // (HIGH = anti-clockwise / LOW = clockwise)
          for (int x = 1; x < StepsToTake; x++) 
          {
              digitalWrite(step_pin, HIGH);
              delay(1);
              digitalWrite(step_pin, LOW);
              delay(1); 
            
            }
            StepperPosition=StepperPosition-StepsToTake;
        }
      }
      else {
        while (StepperPosition != 0)
        { 
          digitalWrite(dir_pin, LOW);  // (HIGH = anti-clockwise / LOW = clockwise)
              for (int x = 1; x < StepsToTake; x++) 
              {
              digitalWrite(step_pin, HIGH);
              delay(1);
              digitalWrite(step_pin, LOW);
              delay(1);  
         
            }
           StepperPosition=StepperPosition+StepsToTake;
        }
      }
      StepperPosition=0; // Reset position to ZERO after moving motor back
    }
  }
*/
// Runs if rotation was detected
  if (TurnDetected)  
  {
        TurnDetected = false;  // do NOT repeat IF loop until new rotation detected

// Which direction to move Stepper motor
        //if (rotationdirection) 
        if(counter>0) 
        { // Move motor CCW
            digitalWrite(dir_pin, HIGH);  // (HIGH = anti-clockwise / LOW = clockwise)
            while (counter != 0)
            {
            for (int x = 1; x < StepsToTake; x++) 
            {
              digitalWrite(step_pin, HIGH);
              delay(1);
              digitalWrite(step_pin, LOW);
              delay(1); 
           
            }
            counter --;

            } 
           Serial.print("---   : ");
           Serial.println(counter);
            StepperPosition=StepperPosition-StepsToTake;
        }
        else
        if(counter<0) 
        { // Move motor CW
            digitalWrite(dir_pin, LOW);  // (HIGH = anti-clockwise / LOW = clockwise)
            while (counter != 0)
            {
            for (int x = 1; x < StepsToTake; x++) 
            {
              digitalWrite(step_pin, HIGH);
              delay(1);
              digitalWrite(step_pin, LOW); 
              delay(1);  
                     
            }
            counter ++;
            } 
            StepperPosition=StepperPosition+StepsToTake;
           Serial.print("+++   : ");
           Serial.println(counter);
        }


  }
}
