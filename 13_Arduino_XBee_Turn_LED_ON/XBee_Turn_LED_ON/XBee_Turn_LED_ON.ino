//XBee Basics - Lesson 5 - API mode/ Send Digital Output to a Rmote XBee - 720p.mp4
// constants won't change. They're used here to 
// set pin numbers:
const int buttonPin = 7;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin
// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int buttonStateOld = 0;
void setup()
{
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);      
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT); 
  Serial.begin(9600);
}
void loop()
{
   // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if(buttonStateOld !=buttonState)
  {
    if (buttonState == HIGH) 
    {     
      // turn LED on:    
        digitalWrite(ledPin, HIGH);
        setRemoteState(0x5); 
        delay(3000);
        // turn LED off:
        digitalWrite(ledPin, LOW);
        setRemoteState(0x4);
    } 
  }
  
}

void setRemoteState(char value)
{
  Serial.write(0x7E);        //0 - start byte
  Serial.write((byte)0x0);   //1 - high part of lenght (always zero)
  Serial.write(0x10);//2 - low part of lenght (the number of bytes that follow, not including the checksum)
  Serial.write(0x17);//3 - 0x17 is a remote AT command
  Serial.write((byte)0x0);//4 - frame id set to zero for no replay
  //ID of recipient, or use 0xFFFF for broadcast
  //5->12 Hex address of destination
 
  Serial.write((byte)0x0);//5 - 64-bit Destination Address (Serial Number)
  Serial.write((byte)0x0);//6 - 
  Serial.write((byte)0x0);//7 - 
  Serial.write((byte)0x0);//8 - 
  Serial.write((byte)0x0);//9 - 
  Serial.write((byte)0x0);//10 -
  Serial.write(0xFF);//11 - 0xFF for broadcast
  Serial.write(0xFF);//12 - 0xFF for broadcast
  //16 bits of recipient or 0xFFFE for broadcast
  Serial.write(0xFF);//13 - 
  Serial.write(0xFE);//14 -
 
  //Remote command option, 0x02 to apply changes 
  Serial.write(0x02);//15 - Remote command option, 0x02 to apply changes
  Serial.write('D');//16 - "D" fromAT Command: D3
  Serial.write('3');//17 - "3" fromAT Command: D3
  //command data in as many bytes as needed
  Serial.write(value); //18 -   0x05 for HIGH, 0x04 for LOW
  
  long checksum = 0x17 + 0xFF + 0xFF +0xFF + 0xFE + 0x02 + 'D' + '3' + value;
  
  Serial.write( 0xFF- (checksum & 0xFF)); 
}
