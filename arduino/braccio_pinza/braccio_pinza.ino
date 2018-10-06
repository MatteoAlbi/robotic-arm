//1900071012001800
//3120314531303050
//--IMPORT
#include <math.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <avr/interrupt.h>

#define Map(n,a,b,x,y) ((n-a)*(y-x)/(b-a)+x)


//--VARIABILI GENERALI
typedef int Matrix[3][4];
bool boolDebug = false;

//variabili gestione radiocomando
volatile int index = 0;
volatile unsigned long prev_time = 0;
const float vel_commands = 1;
const float middle[3] = {1.507,1.511,1.511};
const float rangeCoordinates[3][2] = {{-100,100},
                                      {0,200},
                                      {-100,100}};
volatile Matrix commands = {{1500, 1500, 1000, 1500},
                            {1500, 1500, 1000, 1500},
                            {1500, 1500, 1000, 1500}};
volatile bool rising;
float f_commands[3] = {0, 0, 0};
float coeff[3] = {0.4, 0.3, 0.3};
int RX_pin[4] = {9,10,11,12};

//variabili gestione motori
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
float coordinates[3] = {120,0,120}; //coordinate end effector xyz
float angleTarget[4], motor[4], target[4], gapPerCycle[4], speedMotors[4];//[rad],[pwm],[pwm],[pwm/cycle],[pwm/rad]variazione bit per ciclo, rapporto pwm/gradi

float cycles;//cicli di variazione della pwm
const float mSecPerDeg = 2.666666667;//velocità motore in [deg/msec]
const int pwmMotors[4][3] = {570, 1810, 3080,//valori pwm per ogni motore per angoli 0,90,180
                             730, 1930, 3100,
                             630, 1860, 3080,
                             350, 1550, 2830};
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

void check_coordinates(){
  float temp_coordinates[3];
  for(int i=0;i<3;i++){
    temp_coordinates[i] = coordinates[i]+(f_commands[i]/1000-middle[i])*vel_commands;
  }
  float a = temp_coordinates[0], b = temp_coordinates[1], c = temp_coordinates[2];
  if(a<230 && b<230 && c<230 && sqrt(sq(a)+sq(b)+sq(c))<230) 
    for(int i=0;i<3;i++)
      coordinates[i] += (f_commands[i]/1000-middle[i])*vel_commands; 
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
  for(index = 0; index < 4; index++){
    pinMode(RX_pin[index], INPUT);
  }
  
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
  for(int i = 0; i<4; i++){
    f_commands[i] = 0;
    for(int j = 0; j<3; j++){
      f_commands[i] += commands[j][i]*coeff[j];
    }
    
  }
  check_coordinates();
  
  Serial.println("X: "+String(f_commands[0])
             +"   Y: "+String(f_commands[1])
             +"   Z: "+String(f_commands[2])
             +"   P: "+String(f_commands[3]));
  /*
  Serial.println("X: "+String(coordinates[0])
             +"   Y: "+String(coordinates[1])
             +"   Z: "+String(coordinates[2]));*/
  //delay(2000);
  //movimento braccio
  //MOVE();
}

ISR(PCINT0_vect){    // Port B, PCINT8 - PCINT14
  if(digitalRead(RX_pin[index])==1){ //RISING
    prev_time = micros();
    rising = true;
  }
  else if(digitalRead(RX_pin[index])==0 && rising){ //FALLING
    commands[2][index] = commands[1][index];
    commands[1][index] = commands[0][index];
    commands[0][index] = (micros()-prev_time);
    rising = false;
    index = (index+1)%4;
    PCMSK0 = (0b00000010 << index); 
  }
}
