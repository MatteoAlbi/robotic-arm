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
int pulselen = 1920, servonum = 1;
int pwmMotori[4][3] = {570, 1810, 3080,
                       730, 1930, 3100,
                       630, 1860, 3080,
                       300, 1550, 2830}; 

float pwmPerDegFunc(){
  float velTot = 0;
  for(int i=0;i<4;i++){
    velTot += (pwmMotori[i][2]-pwmMotori[i][0])/180.0;
  }
  return velTot / 4.0;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Servos' angles test, values:");
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  pwm.begin();
  
  pwm.setPWMFreq(333);
  pwm.setPWM(servonum, 0, pulselen);
  for(int i=0;i<4;i++){
    for(int j=0;j<3;j++){
      Serial.print(pwmMotori[i][j]);
      Serial.print(" ");
    }
    
    Serial.println();
  }
  Serial.println(pwmPerDegFunc());
  pwm.setPWM(1, 0, pwmMotori[1][1]);
  pwm.setPWM(2, 0, pwmMotori[2][2]);
  pwm.setPWM(3, 0, pwmMotori[3][2]);
  delay(10);
}

void loop() {
  if(Serial.available()>0) delay(1000);
  if(Serial.available()==2){
    int input = Serial.read();
    if(input=='m'){
      servonum = Serial.read()-48;
      Serial.print("motore ");
      Serial.println(servonum);
    }
    else{
      input = Serial.read();
      Serial.println("err input");
    }
  }
  else if(Serial.available()==4){
    int pulselen = 0;
    for(int i=0;i<4;i++){
      int input = Serial.read()-48;
      pulselen += input*pow(10,3-i);
    }
    Serial.println(pulselen);
    if(pulselen>3300) Serial.println("err pwm");
    else{
      if (servonum == 4) pulselen /= 6.66;
      pwm.setPWM(servonum, 0, pulselen);
    }
  }
  else if(Serial.available()>4){
    Serial.println("err len");
    int count = Serial.available();
    for(int i=0;i<count;i++) int input = Serial.read();
  }
}
