//1900071012001800
//3120314531303050
#include <math.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#define Map(n,a,b,x,y) ((n-a)*(y-x)/(b-a)+x)

bool boolDebug = true;

//variabili gestione motori
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
float coordinates[3] = {0,120,120}; //coordinate end effector xyz
float angleTarget[4], motor[4], target[4], gapPerCycle[4], speedMotors[4];//[rad],[pwm],[pwm],[pwm/cycle],[pwm/rad]variazione bit per ciclo, rapporto pwm/gradi

float cycles;//cicli di variazione della pwm
const float mSecPerDeg = 2.666666667;//velocità motore in [deg/msec]
const int pwmMotors[4][3] = {570, 1810, 3080,//valori pwm per ogni motore per angoli 0,90,180
                             730, 1930, 3100,
                             630, 1860, 3080,
                             300, 1550, 2830};
const int defaultMotors[4] = {pwmMotors[0][1],pwmMotors[1][1],pwmMotors[2][2], pwmMotors[3][2]};//posizione default dei motori
const float angleMotors[4][2] = {{0,    PI},//range degli angoli dei motori, riferiti alla cinematica del braccio, in [rad]
                                 {0,    PI},
                                 {-PI,0},
                                 {-PI,0}};

void coordinatesToPwmFunc(){//date le coordinate xyz dell'end effector, trova il valore pwm che devono avere i motori
  if(boolDebug) {
    Serial.print("coordinates: ");
    for(int i=0;i<3;i++) Serial.print(String(coordinates[i])+" ");
    Serial.println();
  }
  angleTarget[0] = atan2(coordinates[1],coordinates[0]);//Alfa
  float Beta_st = atan2(coordinates[2], sqrt(sq(coordinates[0])+sq(coordinates[1])));
  float Beta_nd = acos(sqrt(sq(coordinates[0])+sq(coordinates[1])+sq(coordinates[2]))/240);
  angleTarget[1] = Beta_st + Beta_nd;//Beta
  angleTarget[2] = Beta_nd*(-2);//Gamma
  angleTarget[3] = - M_PI_2 - angleTarget[1] - angleTarget[2];//Delta
  if(boolDebug) {
    Serial.print("angle targets: ");
    for(int i=0;i<4;i++) Serial.print(String(angleTarget[i])+" ");
    Serial.println();
  }
  for(int i=0;i<4;i++){
    target[i] = Map(angleTarget[i], angleMotors[i][0], angleMotors[i][1], pwmMotors[i][0], pwmMotors[i][2]);
    if(boolDebug) Serial.print(
                    String(angleTarget[i])+" "+
                    String(angleMotors[i][0])+" "+
                    String(angleMotors[i][1])+" "+
                    String(pwmMotors[i][0])+" "+
                    String(pwmMotors[i][2])+" "+
                    String(target[i])+'\n'
                  );
  }
}

float cyclesFunc(){//cicli = media degli angoli da percorrere in pwm
  float somma = 0;
  for(int i=0;i<4;i++) somma+=abs(target[i]-motor[i]);
  return somma/4.0;
}

float tDelayFunc(){//delay tra ogni ciclo = sec per percorrere variazione movimento maggiore
  float vel, maxGapPerCycle = max(abs(gapPerCycle[0]),max(abs(gapPerCycle[1]), max(abs(gapPerCycle[2]), abs(gapPerCycle[3]))));

  for(int i=0;i<4;i++) if(maxGapPerCycle == abs(gapPerCycle[i])) vel = speedMotors[i];
  return maxGapPerCycle*mSecPerDeg/vel;
}

void MOVE(){//date le coordinate da raggiungere e le attuali posizioni dei motori, muove il braccio alle coordinate date 
  coordinatesToPwmFunc();//setta i valori target[i]
  cycles = cyclesFunc();//calcola cicli come media dei gap
  if(boolDebug) Serial.println("gapPerCycle");
  for(int i=0; i<4; i++){
    gapPerCycle[i] = (target[i] - motor[i])/cycles;//variazione della pwm di ogni motore per ciclo
    if(boolDebug) Serial.println(gapPerCycle[i]);
  }

  int tDelay = tDelayFunc();//delay tra ogni ciclo di movimento
  
  for(int i=0; i<cycles; i++){//per ogni ciclo
    for(int j=0; j<4; j++){//per ogni motore
      motor[j] += gapPerCycle[j];//calcolo il  nuovo valore della pwm del motore
      pwm.setPWM(j, 0, int(motor[j]));//setto la pwm al nuovo valore
    }
    delay(tDelay+1);//+1msec per margine
  }
  //for(int i=0; i<4; i++) Serial.println(motor[i]);
  Serial.println(200);//valore di conferma avvenuto processo
}

void setup() {
  Serial.begin(9600);
  if(boolDebug) Serial.println("programma per controllo braccio");
  
  pwm.begin();
  pwm.setPWMFreq(333);
  
  for(int i=0;i<4;i++){
    speedMotors[i] = (pwmMotors[i][2]-pwmMotors[i][0])/180.0;//rapporto pwm/deg dei motori
    motor[i] = defaultMotors[i];
    pwm.setPWM(i, 0, motor[i]);//posizionamento motori su posizioni di default
  }
}

void loop() {
  //acquisizione coordinate
  if(Serial.available()==9){//acquisizione dati(9 cifre, 3 per coordinata)
    for(int i=0; i<3; i++){
      coordinates[i] = 0;
      for(int j=0; j<3; j++){
        int input = Serial.read()-48;
        coordinates[i] += input*pow(10,2-j);
      }
    }
    //movimento braccio
    MOVE();
  }
  else if(Serial.available()>9){
    Serial.println("err len");
    int count = Serial.available();
    for(int i=0;i<count;i++) int input = Serial.read();
  }
}
