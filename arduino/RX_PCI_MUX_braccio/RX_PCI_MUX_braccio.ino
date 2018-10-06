#include <avr/interrupt.h>

typedef int Matrix[3][3];

volatile int index = 0;
volatile unsigned long prev_time = 0;
volatile Matrix commands = {{1500, 1500, 1500},
                             {1500, 1500, 1500},
                             {1500, 1500, 1500}};
volatile bool rising;
int f_commands[4] = {0, 0, 0};
float coeff[3] = {0.4, 0.3, 0.3};
int RX_pin[4] = {9,10,11};
int i,j;

void setup() {
  noInterrupts();
  PCICR = 0b00000001;     // turn on port B
  PCMSK0 = 0b00000010;    // turn on pin PCINT9
  for(index = 0; index < 3; index++){
    pinMode(RX_pin[index], INPUT);
  }
  rising = false;
  index = 0;
  Serial.begin(9600);
  interrupts();
}

void loop() {
  for(i = 0; i<3; i++){
    f_commands[i] = 0;
    for(j = 0; j<3; j++){
      f_commands[i] += commands[j][i]*coeff[j];
    }
  }
  Serial.println("X: "+String(f_commands[0])
             +"   Y: "+String(f_commands[1])
             +"   Z: "+String(f_commands[2]));
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
    index = (index+1)%3;
    PCMSK0 = (0b00000010 << index); 
  }
}

