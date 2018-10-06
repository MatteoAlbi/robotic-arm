void setup() {
  Serial.begin(9600);
}

void loop() {
  if(Serial.available()>0){
    byte val = 0;
    int count = Serial.available();
    for(int i=0; i<count; i++){
      Serial.write(Serial.read());
    }
  }
}
