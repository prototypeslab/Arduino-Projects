/* MLX90316 Rotary Position Sensor
 * KHM 2010 /  Martin Nawrath
 * Kunsthochschule fuer Medien Koeln
 * Academy of Media Arts Cologne
 */

#include <Metro.h> //Include Metro library
#include <MLX90316.h>


int pinSS = 5;
int pinSCK = 3;
int pinMOSI = 4;
int ii;

Metro mlxMetro = Metro(5); 
MLX90316 mlx_1  = MLX90316(); 

void setup(){

  Serial.begin(115200);
  mlx_1.attach(pinSS,pinSCK, pinMOSI ); 
  Serial.println(" MLX90316 Rotary Position Sensor");
}

void loop() {
  if (mlxMetro.check() == 1) { 
  ii = mlx_1.readAngle();
  Serial.print(ii);
  Serial.println("");
  }
}





