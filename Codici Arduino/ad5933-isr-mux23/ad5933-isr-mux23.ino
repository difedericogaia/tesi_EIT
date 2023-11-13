/*
ad5933-multiplexer
    Reads impedance values from the AD5933 over I2C and prints them serially at the end of each cycle of injection. 
    Protocol injection: adjacent (40 measurements) or opposite (32 measurements).
*/

#include <Wire.h>
#include "AD5933.h"

#define START_FREQ  (80000) //80kHz funziona con f_ISR=1Hz
#define FREQ_INCR   (10000)
#define NUM_INCR    (1)
#define REF_RESIST  (500)
#define SIZE (8) //number of electrodes
typedef union {
  float floatingPoint;
  byte binary[4];
} binaryFloat;
int vect[SIZE];
double gain[NUM_INCR+1];
int phase[NUM_INCR+1];
int data=1;
int index=0;
int x=0;
int count=0;
int count_isr=0;
int stop_flag=0;
int reale[1], imaggi[1];
float impedance_measurement_adj[SIZE*5];
const int selectPins_1[3] = {2,3,4}; // S0~2, S1~3, S2~4
const int selectPins_2[3] = {5,6,7}; // S0~5, S1~6, S2~7
const int selectPins_3[3] = {8,9,10}; // S0~8, S1~9, S2~10
const int selectPins_4[3] = {11,12,13}; // S0~11, S1~12, S2~13

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

  //f=0.5 Hz -> T=2s FUNZIONA
  //OCR1A = 31249;
  
  //f=1Hz -> T=1s NON FUNZIONA->funzione con f>=80kHz
  OCR1A = 15624;
  //f=2Hz -> T=0.5s NON FUNZIONA->Funziona 50kHz ma attenzione a cosa stampa...
  //OCR1A = 7812;
  //f=4Hz -> NON FUNZIONA
  //OCR1A = 3906;
  
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
  for (int i=0; i<3; i++)
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
  digitalWrite(15, HIGH);
  
  //Serial.println("ADJACENT PROTOCOL: 'a' ");
  //Serial.println("OPPOSITE PROTOCOL: 'b' ");
  
  // Begin serial at 9600 baud for output
  Serial.begin(9600);
  Serial.println("AD5933 Test Started!");

  //select MUX for calibration: current injection 0-4, voltage measurement 1-5
  selectMuxPin_1(0);
  selectMuxPin_2(4);
  selectMuxPin_3(1);
  selectMuxPin_4(2);
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
          Serial.println(count_isr);
          //count=0;
          index=0;
          for(int i=0;i<SIZE;i++){
            selectMuxPin_1(i);
            selectMuxPin_2(((i+1)%SIZE));
            x = i+2;
              if(!(MyFrequencySweep(reale,imaggi)))
                 Serial.println("Acquisition failed!");
          }
          
          Serial.print("\n");
          //Serial.print(": ");
          //Serial.write(impedance_measurement_adj[40],160);
          /*for (int j=0; j<SIZE*10; j=j+2){
           Serial.print(impedance_measurement_adj[j]);
           if(j==SIZE*10-2){
            Serial.print(";\n");
           }
           else{
            Serial.print(",");
           }
         }*/
        //Serial.println("END!");
               
    data=0;

    /*count++;
    if(count==15){
      Serial.println("END!");
      count=0;
      //stop_flag=1;
    }*/
   }

  //cli();
  
}

void selectMuxPin_1(byte pin)
{
  if (pin > 7) return; // Exit if pin is out of scope
  for (int i=0; i<3; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins_1[i], HIGH);
    else
      digitalWrite(selectPins_1[i], LOW);
  }
}
void selectMuxPin_2(byte pin)
{
  if (pin > 7) return; // Exit if pin is out of scope
  for (int i=0; i<3; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins_2[i], HIGH);
    else
      digitalWrite(selectPins_2[i], LOW);
  }
}
void selectMuxPin_3(byte pin)
{
  if (pin > 7) return; // Exit if pin is out of scope
  for (int i=0; i<3; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins_3[i], HIGH);
    else
      digitalWrite(selectPins_3[i], LOW);
  }
}
void selectMuxPin_4(byte pin)
{
  if (pin > 7) return; // Exit if pin is out of scope
  for (int i=0; i<3; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins_4[i], HIGH);
    else
      digitalWrite(selectPins_4[i], LOW);
  }
}
bool MyFrequencySweep(int real[5], int imag[5]) {
  if (!(AD5933::setPowerMode(POWER_STANDBY) &&         // place in standby
           AD5933::setControlMode(CTRL_INIT_START_FREQ) && // init start freq
           AD5933::setControlMode(CTRL_START_FREQ_SWEEP))) // begin frequency sweep
           {
               return false;
           }
        for(int n=0; n<SIZE-3; n++){
          selectMuxPin_3(x%SIZE);
          selectMuxPin_4((x+1)%SIZE);
          x = x+1;
              // Get the data for this frequency point and store it in the array
              if (!AD5933::getComplexData(&real[0], &imag[0])) {
                  return false;
              }
                  Serial.print(real[0]);
                  Serial.print(",");
                  Serial.print(imag[0]);
                  Serial.print(";");
               //real[0]=0;
               //imag[0]=0;
          // Increment the frequency and our index.

          //setControlMode(CTRL_INCREMENT_FREQ);
      }
      return AD5933::setPowerMode(POWER_STANDBY);
   
  
  
      // Put into standby
   
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
