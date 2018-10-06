//1900071012001800
//3120314531303050
//--IMPORT
#include <math.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <avr/interrupt.h>

#define Map(n,a,b,x,y) ((n-a)*(y-x)/(b-a)+x)


//--VARIABILI GENERALI
typedef int Matrix[3][3];
bool boolDebug = false;

//variabili gestione radiocomando
volatile int index = 0;
volatile unsigned long prevTime = 0;
const float velCommands = 1;
const float rangeCoordinates[3][2] = {{-100,100},
                                      {0,200},
                                      {-100,100}};
volatile Matrix commands = {{1500, 1500, 1500},
                            {1500, 1500, 1500},
                            {1500, 1500, 1500}};
volatile bool rising;
float fCommands[3] = {0, 0, 0};
float coeff[3] = {0.4, 0.3, 0.3};
int RXPin[3] = {9,10,11};

//variabili gestione motori
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

const int trig = 4, echo = 3;//PIN sensore di prossimità
const float minD = 10.3;//valori sensore di prossimità x distanza pendown[cm]
const float downCoordinates = 0.25, upCoordinates = 30.0;//[mm]
const float mSecPerDeg = 2.666666667;//velocità motore in [deg/msec]

bool penUp = true;
float cycles;//cicli di variazione della pwm
float coordinates[3] = {60,0,downCoordinates}; //coordinate end effector xyz
float angleTarget[4],//[rad], angolo che devono avere motori
      motor[4],//[pwm], pwm attuale motori
      target[4],//[pwm], pwm da raggiungere x arrivare ad angolTarget
      gapPerCycle[4],// [pwm/cycle],variazione bit per ciclo,
      speedMotors[4];//[pwm/rad]rapporto pwm/gradi in base a motore

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
  return somma/4.0*5.0;
}

float tDelayFunc(){//delay tra ogni ciclo = sec per percorrere variazione movimento maggiore
  float vel, maxGapPerCycle = max(abs(gapPerCycle[0]),max(abs(gapPerCycle[1]), max(abs(gapPerCycle[2]), abs(gapPerCycle[3]))));

  for(int i=0;i<4;i++) if(maxGapPerCycle == abs(gapPerCycle[i])) vel = speedMotors[i];
  return maxGapPerCycle*mSecPerDeg/vel;
}

float checkDistance(){//calcola distanza con sensore di prossimità
  float duration, distance, z;
  digitalWrite(trig, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
  distance = (duration/2.0) / 29.1;
  if (distance >= 200 || distance <= 0) distance = 9999;
  
//  if(distance > minD){
//    if(downCoordinates-2 < coordinates[2] && coordinates[2] < downCoordinates+2) z = coordinates[2]-2;
//    else if(coordinates[2]>=downCoordinates+2) z = downCoordinates;
//    else z = downCoordinates-2;
//  }
//  else if(distance < minD-0.5){
//    if(downCoordinates-2 < coordinates[2] && coordinates[2] < downCoordinates+2) z = coordinates[2]+2;
//    else if(coordinates[2]<=downCoordinates-2) z = downCoordinates;
//    else z = downCoordinates+2;
//  }
//  else z = coordinates[2];
//
//  return z;

  if (minD-1<distance&&distance<minD) return coordinates[2];
  if (distance>minD) return coordinates[2]-0.5;
  else return coordinates[2]+0.5;
  
  
}

void checkCoordinates(){//controllo su coordinate per mantenerle nel campo di lavoro del braccio
  float tempCoordinates[3];
  for(int i=0;i<2;i++){
    if(abs(fCommands[i]-1500)>50) tempCoordinates[i] = coordinates[i]+(fCommands[i]/1000-1.5)*velCommands;
    else tempCoordinates[i] = coordinates[i];
  }
  tempCoordinates[2] = coordinates[2];
  
  if(fCommands[2]<1300) penUp = true, tempCoordinates[2] = upCoordinates;
  else if(fCommands[2]>1700) penUp = false, tempCoordinates[2] = downCoordinates;
  if(!penUp) tempCoordinates[2] = checkDistance();
  
  float a = tempCoordinates[0], b = tempCoordinates[1], c = tempCoordinates[2];
  if(a<230 && b<230 && c<230 && sqrt(sq(a)+sq(b)+sq(c))<240){
    for(int i=0;i<3;i++) coordinates[i] = tempCoordinates[i];
  }
    
}

void MOVE(){//date le coordinate da raggiungere e le attuali posizioni dei motori, muove il braccio alle coordinate date 
  coordinatesTooPwmFunc();//setta i valori target[i]
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
  //Serial.println(200);//valore di conferma avvenuto processo
}


void setup() {
  noInterrupts();
  PCICR = 0b00000001;     // turn on port B
  PCMSK0 = 0b00000010;    // turn on pin PCINT9
  for(index = 0; index < 3; index++){
    pinMode(RXPin[index], INPUT);
  }
  pinMode(trig,OUTPUT);
  pinMode(echo,INPUT);
  digitalWrite(trig, LOW);
  rising = false;
  index = 0;
  
  delay(1000);
  
  Serial.begin(9600);
  if(boolDebug) Serial.println("programma per controllo braccio");
  interrupts();
  pwm.begin();
  pwm.setPWMFreq(333);
  
  for(int i=0;i<4;i++){
    speedMotors[i] = (pwmMotors[i][2]-pwmMotors[i][0])/180.0;//rapporto pwm/deg dei motori
    motor[i] = defaultMotors[i];
    pwm.setPWM(i, 0, motor[i]);//posizionamento motori su posizioni di default
  }
}

void loop() {
  //acquisizione coordinate [mm]
  for(int i = 0; i<3; i++){
    fCommands[i] = 0;
    for(int j = 0; j<3; j++){
      fCommands[i] += commands[j][i]*coeff[j];
    }
    
  }
  checkCoordinates();
  /*
  Serial.println("X: "+String(fCommands[0])
             +"   Y: "+String(fCommands[1])
             +"   Z: "+String(fCommands[2]));
  */
  Serial.println("X: "+String(coordinates[0])
             +"   Y: "+String(coordinates[1])
             +"   Z: "+String(coordinates[2]));
  
  //if(millis()%2000==0) Serial.println(check_distance()),Serial.println(coordinates[2]);
  //delay(2000);
  //movimento braccio
  MOVE();
}

ISR(PCINT0_vect){    // Port B, PCINT8 - PCINT14
  if(digitalRead(RXPin[index])==1){ //RISING
    prevTime = micros();
    rising = true;
  }
  else if(digitalRead(RXPin[index])==0 && rising){ //FALLING
    commands[2][index] = commands[1][index];
    commands[1][index] = commands[0][index];
    commands[0][index] = (micros()-prevTime);
    rising = false;
    index = (index+1)%3;
    PCMSK0 = (0b00000010 << index); 
  }
}
