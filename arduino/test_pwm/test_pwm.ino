/*************************************************** 
  This is an example for our Adafruit 16-channel PWM & Servo driver
  Servo test - this will drive 8 servos, one after the other on the
  first 8 pins of the PCA9685

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815
  
  These drivers use I2C to communicate, 2 pins are required to  
  interface.

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(&Wire, 0x40);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  0 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  4095 // this is the 'maximum' pulse length count (out of 4096)
int pulselen = 2047, servonum = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("1 channel Servo test!");

  pwm.begin();
  
  pwm.setPWMFreq(333);
  pwm.setPWM(servonum, 0, pulselen);

  delay(10);
}

void loop() {
  if(Serial.available()>=2){
    char input1 = Serial.read();
    int input2 = Serial.read()-48;
    Serial.print(input1);
    Serial.println(input2);
    if(input1 == '-'){
      pulselen-= input2;
      Serial.println("meno");
    }
    else if(input1 == '+'){
      pulselen+= input2;
      Serial.println("più");
    }
    
    Serial.println(pulselen);
    Serial.println();
    pwm.setPWM(servonum, 0, pulselen);
  }
  delay(10);
}
