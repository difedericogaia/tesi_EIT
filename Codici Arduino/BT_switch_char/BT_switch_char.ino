#include <SoftwareSerial.h>
#include <avr/interrupt.h>


SoftwareSerial BTserial(0, 1); // RX | TX

void setup() {
  BTserial.begin(9600);
  delay(10);
  BTserial.println("ready BT...");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (BTserial.available()) {
    switch (BTserial.read()) {
      case 'a':
        BTserial.println("Input char: a");
        break;
      case 'b':
        break;
      case 'c':

        break;
      default:
        break;
    }
}

}
