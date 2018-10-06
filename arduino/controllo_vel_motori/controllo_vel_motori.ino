//1900071012001800
//3120314531303050
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define PWMMIN  520
#define PWMMAX  3300//max pwm val per pwm a 12 bit
//#define SERVOMIN  0//deg min
//#define SERVOMAX  180//max angolo del motore

float motor[4], target[4], gapPerCycle[4];//bit attuale pwm, bit da raggiungere, variazione bit per ciclo
const float mSecPerDeg = 2.67, cycles = 100.0; 
boolean boolDebug = false;

int pwmMotori[4][3] = {660, 1900, 3120,
                       710, 1930, 3145,
                       650, 1900, 3130,
                       570, 1800, 3050}; 


float tDelayFunc(){//delay tra ogni ciclo = sec per percorrere variazione movimento maggiore
  float vel, maxGapPerCycle = max(abs(gapPerCycle[0]),max(abs(gapPerCycle[1]), max(abs(gapPerCycle[2]), abs(gapPerCycle[3]))));
  
  if      (maxGapPerCycle == abs(gapPerCycle[0]))   vel = (pwmMotori[0][2]-pwmMotori[0][0])/180.0;
  else if (maxGapPerCycle == abs(gapPerCycle[1]))   vel = (pwmMotori[1][2]-pwmMotori[1][0])/180.0;
  else if (maxGapPerCycle == abs(gapPerCycle[2]))   vel = (pwmMotori[2][2]-pwmMotori[2][0])/180.0;
  else                                              vel = (pwmMotori[3][2]-pwmMotori[3][0])/180.0;

  return maxGapPerCycle*mSecPerDeg/vel;
}

void MOVE(){
  if(boolDebug) Serial.println("gapPerCycle");
  for(int i=0; i<4; i++){
    gapPerCycle[i] = (target[i] - motor[i])/cycles;//variazione della pwm di ogni motore per ciclo  
    if(boolDebug) Serial.println(gapPerCycle[i]);
  }

  int tDelay = tDelayFunc();
  
  for(int i=0; i<cycles; i++){//per ogni ciclo
    for(int j=0; j<4; j++){//per ogni motore
      motor[j] += gapPerCycle[j];//calcolo il  nuovo valore della pwm del motore
      pwm.setPWM(j, 0, int(motor[j]));//setto la pwm al nuovo valore
    }
    delay(tDelay+1);//+1msec per margine
  }
  //for(int i=0; i<4; i++) Serial.println(motor[i]);
  Serial.println(200);
}

void setup() {
  Serial.begin(9600);
  if(boolDebug) Serial.println("programma per controllo velocità motori");
  
  pwm.begin();
  pwm.setPWMFreq(333);

  for(int i=0;i<4;i++){
    pwm.setPWM(i, 0, pwmMotori[i][2]);
    motor[i] = pwmMotori[i][2];
  }
}

void loop() {
  if(Serial.available()>=16){//acquisizione dati(4 numeri a 4 cifre)
    for(int i=0; i<4; i++){
      target[i] = 0; 
      for( int j=0;j<4;j++) target[i] += (Serial.read()-48) * pow(10,3-j);
    }
    if(boolDebug) for(int i=0;i<4;i++) Serial.print(motor[i]), Serial.print("  "), Serial.println(target[i]);
    MOVE();
  }
}
