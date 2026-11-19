#include <Wire.h>

#include <DS1307.h>
#define TIMEOUT 30 * 1000

void setup() {
 Serial.begin(57600);
}

void loop() {
 if (Serial.available() > 0) {
   char value = Serial.read();
   if (value == 'T') {setTime();}
   if (value == 'R') {sayTime();}
   if (value == 'A') {Serial.println("Hi there");}
 }
}

boolean setTime() {
  //9:55:12 am on Christmas Day, 2009 //09122509065512
Serial.println("TZ");
long startTime = millis();
while (Serial.available() < 14) {
   if (millis() - startTime > TIMEOUT) {
     Serial.println("F");
     return false;
   }
}
 char  twoDigits[2];
 readTwo(DS1307_YR,twoDigits);
  readTwo(DS1307_MTH,twoDigits);
  readTwo(DS1307_DATE,twoDigits);
  readTwo(DS1307_DOW,twoDigits);
  readTwo(DS1307_HR,twoDigits);
  readTwo(DS1307_MIN,twoDigits);
  readTwo(DS1307_SEC, twoDigits);
     Serial.println("T");
     return true;
  

}
 
 void readTwo(int value, char * twoDigits) {
 
   twoDigits[0] = Serial.read();
   twoDigits[1] = Serial.read();
   twoDigits[2] = '\0';
   Serial.println(twoDigits);
   
   RTC.set(value, atoi(twoDigits));

}
 
 void sayTime() {
   Serial.print(RTC.get(DS1307_YR,true));
   Serial.print(" ");
   Serial.print(RTC.get(DS1307_MTH,false));
   Serial.print(" ");
   Serial.print(RTC.get(DS1307_DATE,true));
   Serial.print(" ");
   Serial.print(RTC.get(DS1307_HR,true));
   Serial.print(":");
   Serial.print(RTC.get(DS1307_MIN,true));
       Serial.print(":");
       Serial.println(RTC.get(DS1307_SEC,true));
 }