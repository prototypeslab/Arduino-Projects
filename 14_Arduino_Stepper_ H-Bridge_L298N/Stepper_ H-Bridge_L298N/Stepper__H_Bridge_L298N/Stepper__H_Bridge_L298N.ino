int ledPin = 13;
// motor one
int enA = 10;
int in1 = 9;
int in2 = 8;
// motor two
int enB = 5;
int in3 = 7;
int in4 = 6;
void setup()

{
  pinMode (ledPin, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
}
void mydelay(int clk)
{
  int i;
  for (i=0;i< clk;i++);
  //delay(clk);
}
class StepperL298N
{
  public:
  byte pins[4];
  byte curState;
  byte* lut;
  StepperL298N(byte pin1,byte pin2,byte pin3,byte pin4);
  
  void step(boolean dir);
  void halfStep(boolean yes) {if (yes) lut =
  halfStepLut; else lut = fullStepLut;}
  void start() { curState = 0xa;}
  void free() { curState = 0; }
  void brake() {curState = 0;}
  byte fullStepLut[16];
  byte halfStepLut[16];
};
StepperL298N::StepperL298N(byte pin1,byte pin2,byte pin3,byte pin4)
{
  pins[0] = pin1; pins[1] = pin2; pins[2]=pin3; pins[3] = pin4;
  lut = fullStepLut;
  for (int i=0;i<4;i++)
  {
    pinMode(pins[i], OUTPUT);
  }
  memset(&fullStepLut,sizeof(byte)*16,0);
  memset(&halfStepLut,sizeof(byte)*16,0);
  // high nibble goes one way, low the other
  fullStepLut[0] = 0;
  // 1010 -> 1001 -> 0101 -> 0110
  fullStepLut[0xa] = 0x9 | 0x60;
  fullStepLut[0x9] = 0x5 | 0xa0;
  fullStepLut[0x5] = 0x6 | 0x90;
  fullStepLut[0x6] = 0xa | 0x50;
  halfStepLut[0] = 0;
  // 1010 -> 1000 -> 1001 -> 0001 -> 0101 -> 0100 -> 0110 -> 0010
  halfStepLut[0xa] = 0x8 | 0x20;
  halfStepLut[0x8] = 0x9 | 0xa0;
  halfStepLut[0x9] = 0x1 | 0x80;
  halfStepLut[0x1] = 0x5 | 0x90;
  halfStepLut[0x5] = 0x4 | 0x10;
  halfStepLut[0x4] = 0x6 | 0x50;
  halfStepLut[0x6] = 0x2 | 0x40;
  halfStepLut[0x2] = 0xa | 0x60;
}
void StepperL298N::step(boolean dir)
{
curState = lut[curState];
if (dir) curState &= 0xf;
else curState >>= 4;
for (int i=1, j=0;i< 16;i*= 2,j++)
{
  byte lvl = LOW;
  if (i&curState) lvl=HIGH;
  digitalWrite(pins[j],lvl);
}
}
int serDateVel = LOW;
void loop()

{
  unsigned long int j;
  digitalWrite(enA, 1);
  digitalWrite(enB, 1);
  
  StepperL298N motor(6,7,8,9);
  motor.halfStep(false);
  motor.start();
  for (j=0;j< 200;j++)
  {
    motor.step(0);
    digitalWrite(ledPin, j&1);
    //delay(30+j/1000); // jo højere tal jo lavere fart
    delay(10);
  }
  digitalWrite(enA, 0);
  digitalWrite(enB, 0);
  delay(1000);
 /* 
  motor.halfStep(false);
  motor.start();
  for (j=0;j< 100;j++)
  {
    motor.step(0);
    digitalWrite(ledPin, j&1);
    //delay(30+j/1000); // jo højere tal jo lavere fart
    delay(50);
  }
  delay(5000);
  
  StepperL298N motor2(6,7,8,9);
  motor2.halfStep(false);
  motor2.start();
  for (j=0;j< 1000;j++)
  {
    motor2.step(1);
    digitalWrite(ledPin, j&1);
    delay(30+j/1000); // jo højere tal jo lavere fart
    //delay(50);
  }
  motor2.halfStep(false);
  motor2.start();
  for (j=0;j< 1000;j++)
  {
    motor2.step(0);
    digitalWrite(ledPin, j&1);
    delay(30+j/1000); // jo højere tal jo lavere fart
  }
 StepperL298N motor3(6,7,8,9);
  motor3.halfStep(false);
  motor3.start();
  for (j=0;j< 1000;j++)
  {
    motor3.step(1);
    digitalWrite(ledPin, j&1);
    delay(30+j/1000); // jo højere tal jo lavere fart
    //delay(50);
  }
  motor3.halfStep(false);
  motor3.start();
  for (j=0;j< 1000;j++)
  {
    motor3.step(0);
    digitalWrite(ledPin, j&1);
    delay(30+j/1000); // jo højere tal jo lavere fart
  }
  */
}

