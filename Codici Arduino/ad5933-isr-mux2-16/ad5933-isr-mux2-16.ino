/*
ad5933-multiplexer
    Reads impedance values from the AD5933 over I2C and prints them serially at the end of each cycle of injection. 
    Protocol injection: adjacent (208 measurements) or opposite (32 measurements).
*/

#include <Wire.h>
#include "AD5933.h"

#define START_FREQ  (80000) //80kHz funziona con f_ISR=1Hz
#define FREQ_INCR   (10000)
#define NUM_INCR    (1)
#define REF_RESIST  (1000)
#define SIZE (16) //number of electrodes
typedef union {
  float floatingPoint;
  byte binary[4];
} binaryFloat;
int vect[SIZE];
double gain[NUM_INCR+1];
int phase[NUM_INCR+1];
int data=1;
int index=0;
int count=0;
int count_isr=0;
int stop_flag=0;
int reale[1], imaggi[1];
float impedance_measurement_adj[SIZE*5];

const int selectPins_1[4] = {13,12,11,10}; // S0~13, S1~12, S2~11, S3=10
const int selectPins_2[4] = {9,8,7,6}; // S0~9, S1~8, S2~7 S3=6
const int selectPins_3[4] = {5,4,3,2}; // S0~5, S1~4, S2~3 S1=2
const int selectPins_4[4] = {14,15,16,17}; // S0~14, S1~15, S2~16 S3=17

void setup(void)
{
  //stop all interrupts
  cli();
  //set timer1 interrupt at 0.5Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  //OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)

  //f=0.5 Hz -> T=2s NON FUNZIONA con 16 elettrodi né con 12
  //OCR1A = 31249;

  //f=0.25 Hz -> funziona con 16 elettrodi, con 12
  OCR1A=62500;

  //f=0.33 Hz NON FUNZIONA con 16, funziona con 12
  //OCR1A = 47350;

  //f=0.4 Hz funziona con 12
  //OCR1A = 39000;

  //f=1Hz -> T=1s NON FUNZIONA->funzione con f>80kHz, 8 elettrodi, con 12 non funziona
  //OCR1A = 15624;
  //f=2Hz -> T=0.5s NON FUNZIONA con 12
  //OCR1A = 7812;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  //allow interrupt
  sei();

  
  // Begin I2C
  Wire.begin();
  for(int i=0; i<SIZE; i++){
    vect[i]=i;
  }
    //initialize MUX pins as output
  for (int i=0; i<4; i++)
  {
    pinMode(selectPins_1[i], OUTPUT);
    digitalWrite(selectPins_1[i], LOW);
    pinMode(selectPins_2[i], OUTPUT);
    digitalWrite(selectPins_2[i], LOW);
    pinMode(selectPins_3[i], OUTPUT);
    digitalWrite(selectPins_3[i], LOW);
    pinMode(selectPins_4[i], OUTPUT);
    digitalWrite(selectPins_4[i], LOW);
  }
  

  //EN MUX
  //digitalWrite(15, HIGH);
  
  //Serial.println("ADJACENT PROTOCOL: 'a' ");
  //Serial.println("OPPOSITE PROTOCOL: 'b' ");
  
  // Begin serial at 9600 baud for output
  Serial.begin(9600);
  Serial.println("AD5933 Test Started!");

  //select MUX for calibration: current injection 0-4, voltage measurement 1-5
  selectMuxPin_1(0);
  selectMuxPin_2(1);
  selectMuxPin_3(2);
  selectMuxPin_4(3);
  delay(1);
  
  // Perform initial configuration. Fail if any one of these fail.

  if (!(AD5933::reset() &&
        AD5933::setInternalClock(true) &&
        AD5933::setStartFrequency(START_FREQ) &&
        AD5933::setIncrementFrequency(FREQ_INCR) &&
        AD5933::setNumberIncrements(NUM_INCR) &&
        AD5933::setPGAGain(PGA_GAIN_X1)))
        {
            Serial.println("FAILED in initialization!");
            while (true) ;
        }

  // Perform calibration sweep
  if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR+1))
    Serial.println("Calibrated!");
  else
    Serial.println("Calibration failed...");


  delay(1000);
  Serial.println("Acquisition starts in 1 second...");
  delay(1000);
}

void loop(void)
{
  
   if(data==1){
          //Serial.print("NUmero isr: ");
          //Serial.print(count_isr);
          //Serial.print("\n");
          //count=0;
          index=0;
          for(int i=0;i<SIZE;i++){
            selectMuxPin_1(i);
            selectMuxPin_2(((i+1)%SIZE));
            int x = i+2;
            for(int n=0; n<SIZE-3; n++){
              selectMuxPin_3(x%SIZE);
              selectMuxPin_4((x+1)%SIZE);
              x = x+1;
              //Serial.println(x);
              if (AD5933::frequencySweepSingle(reale, imaggi)) {
                  double magnitude = sqrt(pow(reale[0], 2) + pow(imaggi[0], 2));
                  double impedance = 1/(magnitude*gain[0]);;
                  //Serial.write(magnitude.binary,4);
                  /*Serial.print(reale[0]);
                  Serial.print(",");
                  Serial.print(imaggi[0]);
                  Serial.print(";");*/
                  //impedance_measurement_adj[index]=impedance;
                  Serial.print(impedance);
                  Serial.print(",");
                  //index++;
                            
              }
              else Serial.println("Acquisition failed!");
            }
          }
          Serial.print(";\n");
          /*
          Serial.print(count);
          Serial.print(": ");
          for (int j=0; j<SIZE*5; j=j+1){
           Serial.print(impedance_measurement_adj[j]);
           if(j==SIZE*5-1){
            Serial.print(";\n");
           }
           else{
            Serial.print(",");
           }
         }
        //Serial.println("END!");
        */
    data=0;

    count++;
    if(count==15){
      //Serial.println("END!");
      count=0;
      //stop_flag=1;
    }
   }

  //cli();
  
}


void selectMuxPin_1(byte pin)
{
  if (pin > 15) return; // Exit if pin is out of scope
  for (int i=0; i<4; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins_1[i], HIGH);
    else
      digitalWrite(selectPins_1[i], LOW);
  }
}
void selectMuxPin_2(byte pin)
{
  if (pin > 15) return; // Exit if pin is out of scope
  for (int i=0; i<4; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins_2[i], HIGH);
    else
      digitalWrite(selectPins_2[i], LOW);
  }
  //Serial.println("OK MUX 2");
}
void selectMuxPin_3(byte pin)
{
  if (pin > 15) return; // Exit if pin is out of scope
  for (int i=0; i<4; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins_3[i], HIGH);
    else
      digitalWrite(selectPins_3[i], LOW);
  }
}
void selectMuxPin_4(byte pin)
{
  if (pin > 15) return; // Exit if pin is out of scope
  for (int i=0; i<4; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins_4[i], HIGH);
    else
      digitalWrite(selectPins_4[i], LOW);
  }
}


ISR(TIMER1_COMPA_vect){ //timer1 interrupt 0.5 Hz 
//
    count_isr++;
    if (data==0) {
      if(stop_flag==0){
        data=1;
      }
    }
    //else data=0;
}
/*
ISR(TIMER1_COMPA_vect){ //timer1 interrupt 0.5 Hz 
//
    count_isr++;
    data=1;
    //else data=0;
}*/
