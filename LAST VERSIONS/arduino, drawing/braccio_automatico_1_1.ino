//1900071012001800
//3120314531303050
//--IMPORT
#include <math.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define Map(n,a,b,x,y) ((n-a)*(y-x)/(b-a)+x)


//--VARIABILI GENERALI
bool boolDebug = false;

//variabili gestione motori
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

const int trig = 4, echo = 3;//PIN sensore di prossimità
const int cyclesMultiplier = 5;
const float minD = 9.4;//valori sensore di prossimità x distanza pendown[cm]
const float downCoordinates = 0.25, upCoordinates = 30.0;//[mm]
const float mSecPerDeg = 2.666666667;//velocità motore in [deg/msec]

bool penUp = true, finish = false;
float cycles;//cicli di variazione della pwm
float angleTarget[4],//[rad], angolo che devono avere motori
      motor[4],//[pwm], pwm attuale motori
      target[4],//[pwm], pwm da raggiungere x arrivare ad angolTarget
      gapPerCycle[4],// [pwm/cycle],variazione bit per ciclo,
      speedMotors[4],//[pwm/rad]rapporto pwm/gradi in base a motore
      defaultCoordinates[3] = {80,0,upCoordinates},//[mm]
      coordinates[3] = {80,0,upCoordinates}; //coordinate end effector xyz[mm]
int   commands[20];//[mm]

const int pwmMotors[4][3] = {570, 1810, 3080,//valori pwm per ogni motore per angoli 0,90,180
                             730, 1930, 3100,
                             630, 1860, 3080,
                             400, 1550, 2830};
const int defaultMotors[4] = {pwmMotors[0][1],pwmMotors[1][1],pwmMotors[2][2], pwmMotors[3][2]};//posizione default dei motori
const float angleMotors[4][2] = {{-M_PI_2,  M_PI_2},//range degli angoli dei motori, riferiti alla cinematica del braccio, in [rad]
                                 {0,        M_PI},
                                 {-M_PI,    0},
                                 {-M_PI,    0}};



//--FUNZIONI                      
void coordinatesTooPwmFunc(){//date le coordinate xyz dell'end effector, trova il valore pwm che devono avere i motori
  angleTarget[0] = atan2(coordinates[1],coordinates[0]);//Alfa
  float Beta_st = atan2(coordinates[2], sqrt(sq(coordinates[0])+sq(coordinates[1])));
  float Beta_nd = acos(sqrt(sq(coordinates[0])+sq(coordinates[1])+sq(coordinates[2]))/240);//[mm]
  angleTarget[1] = Beta_st + Beta_nd;//Beta
  angleTarget[2] = Beta_nd*(-2);//Gamma
  angleTarget[3] = - M_PI_2 - angleTarget[1] - angleTarget[2];//Delta

  for(int i=0;i<4;i++){
    target[i] = Map(angleTarget[i], angleMotors[i][0], angleMotors[i][1], pwmMotors[i][0], pwmMotors[i][2]);
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

void checkDistance(){//calcola distanza con sensore di prossimità [cm]
  float duration, distance;
  digitalWrite(trig, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
  distance = (duration/2.0) / 29.1;
  if (distance >= 200 || distance <= 0) distance = 9999;
  Serial.write(int(distance*10));

  if (minD-0.5<=distance&&distance<=minD+0.5);
  else if (distance>minD) coordinates[2]-=0.5, MOVE();
  else coordinates[2]+=0.2, MOVE();
}

void MOVE(){//date le coordinate da raggiungere e le attuali posizioni dei motori, muove il braccio alle coordinate date 
  coordinatesTooPwmFunc();//setta i valori target[i]
  cycles = cyclesFunc()*cyclesMultiplier;//calcola cicli come media dei gap
  for(int i=0; i<4; i++){
    gapPerCycle[i] = (target[i] - motor[i])/cycles;//variazione della pwm di ogni motore per ciclo
  }

  int tDelay = tDelayFunc();//delay tra ogni ciclo di movimento
  
  for(int i=0; i<cycles; i++){//per ogni ciclo
    for(int j=0; j<4; j++){//per ogni motore
      motor[j] += gapPerCycle[j];//calcolo il  nuovo valore della pwm del motore
      pwm.setPWM(j, 0, int(motor[j]));//setto la pwm al nuovo valore
    }
    delay(tDelay+1);//+1msec per margine
  }
  if(!penUp) checkDistance();
}


void setup() {
  pinMode(trig,OUTPUT);
  pinMode(echo,INPUT);
  digitalWrite(trig, LOW);
  delay(1000);
  
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(333);
  
  for(int i=0;i<4;i++){
    speedMotors[i] = (pwmMotors[i][2]-pwmMotors[i][0])/180.0;//rapporto pwm/deg dei motori
    motor[i] = defaultMotors[i];
    pwm.setPWM(i, 0, motor[i]);//posizionamento motori su posizioni di default
  }
  MOVE();
  delay(3000);
  Serial.write(253);
}

void loop() {
  //acquisizione coordinate [mm]
  if(Serial.available()==1){
    int nCommands = Serial.read();
    Serial.write(nCommands);
    while(Serial.available()!=nCommands);
    Serial.write(252);
    for(int i=0;i<nCommands;i++) commands[i] = Serial.read();
    int iter = 0;
    while(iter<nCommands){
      if(commands[iter] == 254) {
        penUp = true;
        coordinates[2] = upCoordinates;
        MOVE();
        delay(1000);
        Serial.write(commands[iter]);
      }
      else if(commands[iter] == 255) {
        penUp = true;
        coordinates[2] = upCoordinates;
        MOVE();
        delay(1000);
        for(int i=0;i<2;i++) coordinates[i] = defaultCoordinates[i];
        MOVE();
      }
      else{
        coordinates[0] = commands[iter];
        iter++;
        coordinates[1] = commands[iter]-80;
        MOVE();
        if(penUp){
          penUp = false;
          coordinates[2] = downCoordinates;
          MOVE();
        }
      }
      iter++;
    }
    Serial.write(253);//valore di conferma avvenuto processo
  }
}
