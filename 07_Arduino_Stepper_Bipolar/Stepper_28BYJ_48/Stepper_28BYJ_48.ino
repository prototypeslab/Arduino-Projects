/* Example code for stepper motor 28BYJ-48
/Connect the controller pins for In1, In2, In3 and In4, to Arduino pins 8,9,10,11
Connect the controller +ve5 and -ve to Arduino +ve 5v and ground
The Motor draws 90 ma. The USB can supply 5V at 500 ma to the Arduino, 
*/

#include <Stepper.h> // Include the stepper library

const int stepsPerRevolution = 2048; // 2048steps per revolution for the 28BYJ-48
// initialize the stepper library on pins 8 - 11: 
Stepper myStepper(stepsPerRevolution, 8,10,9,11); //note the modified sequence

void setup() 
{
  myStepper.setSpeed(10);
}

void loop() 
{
// set the speed - max 14
  myStepper.step(stepsPerRevolution/2); // step two revolutions in one direction:. 
  delay(5000); // To turn 1/2 revolutions - use /2; etc 

  myStepper.step(-stepsPerRevolution/2); // step one revolution in the other direction: 
  delay(5000);
}

