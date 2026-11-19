#include <Wire.h>
#include <FRAMX.h>
 
/*Initialize the FRAM and use the default 
  FRAM address that's provided */
FRAMX fram(FRAMX::FRAMX_I2C_DEFAULT_ADDRESS);
 
/* read and write buffers here are used to store incoming and outgoing data */
static unsigned char read_buffer[2] ={0x00,0x00};
static unsigned char write_buffer[2]={0x00,0x01};
 
void setup(){
/* we must call this before using any I2C functions! */
    Wire.begin();
    Serial.begin(9600);
}
 
void loop(){
 
    /* memory address to begin with on FRAM */
    unsigned char start_address = 0;
 
    /* do not exceed the length of your read and write buffers */
    unsigned char number_of_bytes_to_transfer = 2;
 
    /* Write some stuff! */
    unsigned int number_of_bytes_written = fram.write(start_address, write_buffer, number_of_bytes_to_transfer);
 
    /* Read some stuff! */
    unsigned int number_of_bytes_read = fram.read(start_address, read_buffer, number_of_bytes_to_transfer);
 
 
    /* Print what we got */
    for(int i=0; i<number_of_bytes_to_transfer; i++){
        Serial.print("Wrote: ");
        Serial.print(write_buffer[i], HEX);
        Serial.print(" Read back: ");
        Serial.print(read_buffer[i], HEX);
        Serial.println(" ");
    }
 
    /* Handle any I2C errors */
    unsigned int error = fram.errors();
    Serial.print("Error code: ");
    Serial.print(error,HEX);
 
    switch(error)
    {
        case 0: Serial.print(" [Success!] "); 
            break;
        case 1: Serial.print(" [Data too large for buffer] "); 
            break;
        case 2: Serial.print(" [I2C Address was not found] "); 
            break;
        case 3: Serial.print(" [Data was not acknowledged] "); 
            break;
        default:Serial.print(" [There was some other error] "); 
            break;
    }
    Serial.println(" ");
 
    delay(5000);
}
