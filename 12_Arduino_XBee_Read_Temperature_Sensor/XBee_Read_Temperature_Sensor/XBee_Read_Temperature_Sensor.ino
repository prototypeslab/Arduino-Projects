//XBee Basics - Lesson 4 - API mode/ Reading analog data from remote XBee - 720p.mp4

float temp;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  // Remote XBee: AT, Base XBee: API
if (Serial.available() >= 21) 
{ 
	if (Serial.read() == 0x7E) 
		{
		// Make sure the frame is all there // 7E is the start byte
		// Skip ahead to the analog data
			for (int i = 1; i<19; i++) 
			{
			byte discardByte = Serial.read();
			}
		int analogMSB = Serial.read(); 
                int analogLSB = Serial.read();
		// Read the first analog byte data int analogLSB = Serial.read(); 
		// Read the second byte
                Serial.println("**********************");
                Serial.print("analogLSB "); Serial.println(analogLSB);
                Serial.print("analogMSB "); Serial.println(analogMSB * 256);
                            
		int analogReading = analogLSB + (analogMSB * 256);
               
                Serial.println("**********************");
               // converting that reading to voltage, for 3.3v arduino use 3.3
               float voltage = analogReading ;
               voltage /= 1024.0; 
               // print out the voltage
               Serial.print(voltage); Serial.println(" volts");
               // now print out the temperature
               float temperatureC = voltage * 125*1.02 -50; 
              
               //converting from 10 mv per degree wit 500 mV offset
               //to degrees ((voltage - 500mV) times 100)
              
               Serial.print(temperatureC); Serial.println(" degrees C");
               
               // now convert to Fahrenheit
               float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
               Serial.print(temperatureF); Serial.println(" degrees F");

                
                
                
                
		} 
}
  
}
