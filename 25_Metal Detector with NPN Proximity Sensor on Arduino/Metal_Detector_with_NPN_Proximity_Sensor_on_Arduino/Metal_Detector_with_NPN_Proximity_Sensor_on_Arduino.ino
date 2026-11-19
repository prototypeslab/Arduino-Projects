/*
14CORE TEST CODE FOR PROXIMITY SENSOR 
Metal Detection with 3 wire sensor
http://www.14core.com
*/

float metalDetected;
int monitoring;
int metalDetection = 1;
int ledPin = 2;

void setup(){
Serial.begin(9600);
 pinMode(ledPin, OUTPUT);
}

void loop(){
monitoring = analogRead(metalDetection);
if (monitoring >0)
{
 digitalWrite(ledPin, HIGH);
}
else
{
  digitalWrite(ledPin, LOW);
}
}
