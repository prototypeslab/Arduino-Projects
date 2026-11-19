/*
 * Project name:
     Stepper Click Board Example
 * Copyright:
     (c) Mikroelektronika, 2014.
 * Revision History:
     20140225:
       - initial release (FJ);
 * Description:
     This is a simple demonstration of the Stepper Click Board usage. Use RD7 RD6, RD5 and RD4 buttons
     to toggle stepper motor resolution. Use RD0 and RD1 arrow buttons to initiate CW or CCW motor rotations.
     You can monitor resolution and rotations on the Lcd.
 * Test configuration:
     MCU:             P18F45K22
                      http://ww1.microchip.com/downloads/en/DeviceDoc/41412F.pdf
     Dev.Board:       EasyPIC v7
                      http://www.mikroe.com/easypic/
     Oscillator:      HS-PLL, 32.000MHz
     Ext. Modules:    Stepper Click Board - ac:Stepper_Click_Board
                      http://www.mikroe.com/click/stepper/
                      LCD 2x16 chars - ac:LCD
                      http://www.mikroe.com/store/components/displays/                     
     SW:              mikroC PRO for PIC
                      http://www.mikroe.com/mikroc/pic/
 * NOTES:
     - Place DC Motor click board at the mikroBUS socket 1 on the EasyPIC v7 board.
     - Bipolar stepper motor connections :
          OUT2A - Blue wire
          OUT1A - Pink wire
          OUT2B - Yellow wire
          OUT1B - Orange wire
          NC    - Red wire
     - Put MOTOR PWR jumper in 5V position.
     - Put power supply jumper (J5) on the EasyPIC7 board in 5V position.    
     - Pull-down PORTD, put J17 in VCC position on the EasyPIC v7 board.
     - Put Lcd in its socket, turn on backlight using the SW4.6, 
         and adjust contrast via P4 potentiometer on the EasyPIC v7 board.
  */

// Bipolar Stepper motor driver connections
sbit DIR  at LATE0_bit;
sbit STEP at LATC0_bit;
sbit MS1 at LATA2_bit;
sbit MS2 at LATE1_bit;

sbit DIR_Direction  at TRISE0_bit;
sbit STEP_Direction at TRISC0_bit;
sbit MS1_Direction at TRISA2_bit;
sbit MS2_Direction at TRISE1_bit;
// End Bipolar Stepper motor driver connections

// LCD module connections
sbit LCD_RS at LATB4_bit;
sbit LCD_EN at LATB5_bit;
sbit LCD_D4 at LATB0_bit;
sbit LCD_D5 at LATB1_bit;
sbit LCD_D6 at LATB2_bit;
sbit LCD_D7 at LATB3_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISB0_bit;
sbit LCD_D5_Direction at TRISB1_bit;
sbit LCD_D6_Direction at TRISB2_bit;
sbit LCD_D7_Direction at TRISB3_bit;
// End LCD module connections
  
unsigned int cnt = 0;

// Routine to execute single step
void Single_Step(){
  STEP = 0;
  Delay_ms(3);
  STEP = 1;
  Delay_ms(3);
}

void main() {
  ANSELA = 0;                    // Configure AN pins as digital
  ANSELC = 0;
  ANSELD = 0;

  // Set Bipolar Stepper motor driver direction
  DIR_Direction = 0;
  STEP_Direction = 0;
  MS1_Direction = 0;
  MS2_Direction = 0;

  // Default start-up configuration
  DIR = 0;                       // Set clockwise direction
  MS1 = 0;                       // Set full step
  MS2 = 0;

  Lcd_Init();                    // Init LCD
  Lcd_Cmd(_LCD_CLEAR);           // Clear display
  Lcd_Cmd(_LCD_CURSOR_OFF);      // Cursor off
  
  // Intro message
  Lcd_Out(1, 1, "Set res. RD7-RD4");
  Lcd_Out(2, 1, "Set rot. RD0,RD1");
  Delay_ms(3000);
  Lcd_Cmd(_LCD_CLEAR);
  
  Lcd_Out(1, 1, "Res:  Full Step");
  Lcd_Out(2, 1, "Rotation: ");

  while (1) {
    // Full Step
    if (Button(&PORTD, 7, 1, 1)) {
      MS1 = 0;
      MS2 = 0;
      Lcd_Out(1, 7, "Full Step");
    }

    // 1/2 Step
    if (Button(&PORTD, 6, 1, 1)) {
      MS1 = 1;
      MS2 = 0;
      Lcd_Out(1, 7, "1/2 Step ");
    }

    // 1/4 Step
    if (Button(&PORTD, 5, 1, 1)) {
      MS1 = 0;
      MS2 = 1;
      Lcd_Out(1, 7, "1/4 Step ");
    }
    
    // 1/8 Step
    if (Button(&PORTD, 4, 1, 1)) {
      MS1 = 1;
      MS2 = 1;
      Lcd_Out(1, 7, "1/8 Step ");
    }

    // Rotate clockwise
    if (Button(&PORTD, 1, 1, 1)) {
      DIR = 0;                   // Set clockwise direction
      Lcd_Out(2, 13, "CW");      // Display message on Lcd
      
      // Rotate 360 degrees
      for (cnt = 0; cnt <= 2038; cnt++) {
        Single_Step();                  // Make single step
        if (Button(&PORTD, 0, 5, 1)) {  // Change rotation direction to counterclockwise
          break;
        }
      }
    }

    // Rotate counterclockwise
    if (Button(&PORTD, 0, 1, 1)){
      DIR = 1;                   // Set counterclockwise direction
      Lcd_Out(2, 12, "CCW");     // Display message on Lcd
      
      // Rotate 360 degrees
      for (cnt = 0; cnt <= 2038; cnt++) {
        Single_Step();                  // Make single step
        if (Button(&PORTD, 1, 5, 1))    // Change rotation direction to clockwise
          break;
      }
      Lcd_Chr(2, 12, ' ');
    }
  }
}