#include <SoftwareSerial.h>

SoftwareSerial BTserial(0, 1); // RX | TX
int a;

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  //delay(1000);
  //Serial.println("ready...");
  BTserial.begin(9600);
  delay(1000);
  BTserial.println("ready BT...");
  a=0;
}

void loop() {
  // put your main code here, to run repeatedly:
  
  BTserial.println("ciao\n");
  delay(1000);
  
}
