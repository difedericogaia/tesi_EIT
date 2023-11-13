/*
ad5933-multiplexer
    16 electrodes setup
    Reads impedance values from the AD5933 over I2C and prints them serially at the end of each cycle of injection. 
    Protocol injection: adjacent (40 measurements) or opposite (32 measurements).
*/

#include <Wire.h>
#include "AD5933.h"

#define START_FREQ  (10000)
#define FREQ_INCR   (10000)
#define NUM_INCR    (9)
#define REF_RESIST  (500)
#define SIZE (16) //number of electrodes

int vect[SIZE];
//int index;
double impedance_measurement_adj[SIZE*10];
double gain[NUM_INCR+1];
int phase[NUM_INCR+1];

const int selectPins_1[4] = {13,12,11,10}; // S0~13, S1~12, S2~11, S3=10
const int selectPins_2[4] = {9,8,7,6}; // S0~9, S1~8, S2~7 S3=6
const int selectPins_3[4] = {5,4,3,2}; // S0~5, S1~4, S2~3 S1=2
const int selectPins_4[4] = {14,15,16,17}; // S0~14, S1~15, S2~16 S3=17


void setup(void)
{
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
  Serial.begin(9600);
  //EN MUX
  //digitalWrite(15, HIGH);
  
  Serial.println("ADJACENT PROTOCOL: 'a' ");
  Serial.println("OPPOSITE PROTOCOL: 'b' ");
  
  // Begin serial at 9600 baud for output

  Serial.println("AD5933 Test Started!");

  //select MUX for calibration: current injection 0-4, voltage measurement 1-5
  //0412,0123
  selectMuxPin_1(0);
  selectMuxPin_2(8);
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
  
}

void loop(void)
{
   if (Serial.available()) {
    switch (Serial.read()) {
      case 'a':
        Serial.println("MUX 1, Valore da scrivere: 1");
        selectMuxPin_1(1);
        delay(1000);
        Serial.println("MUX 2, Valore da scrivere: 1");
        selectMuxPin_2(1);
        delay(1000);
        Serial.println("MUX 3, Valore da scrivere: 1");
        selectMuxPin_3(1);
        delay(1000);
        Serial.println("MUX 4, Valore da scrivere: 1");
        selectMuxPin_4(1);
        delay(1000);
        
           
        break;
      case 'b':
        break;
        

    }
   }
}

// Easy way to do a frequency sweep. Does an entire frequency sweep at once and
// stores the data into arrays for processing afterwards. This is easy-to-use,
// but doesn't allow you to process data in real time.
void frequencySweepAdj() {
    // Create arrays to hold the data
    int real[NUM_INCR+1], imag[NUM_INCR+1];

    // Perform the frequency sweep
    if (AD5933::frequencySweep(real, imag, NUM_INCR+1)) {
      // Print the frequency data
      int cfreq = START_FREQ/1000; 
      for (int i = 0; i < NUM_INCR+1; i++, cfreq += FREQ_INCR/1000) {
        // Print raw frequency data
        
        /*Serial.print(cfreq);
        Serial.print(": ");
        Serial.print(real[i]);
        Serial.print("/I=");
        Serial.print(imag[i]);*/

        // Compute impedance
        double magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
        double impedance = 1/(magnitude*gain[i]);
        //Serial.print("\n  |Z|=");
        Serial.print(impedance);
        if(i==NUM_INCR){
          Serial.print(";");
        }

        else{     
          Serial.print(",");
        }
        //impedance_measurement_adj[index]=impedance;
        //index = index+1;
      }
      //Serial.println("Frequency sweep complete!");
      //Serial.print(";");
      Serial.print("\n");
    } else {
      Serial.println("Frequency sweep failed...");
    }
}


void selectMuxPin_1(byte pin)
{
  if (pin > 15) return; // Exit if pin is out of scope
  for (int i=0; i<4; i++)
  {
    if (pin & (1<<i)){
      digitalWrite(selectPins_1[i], HIGH);
      Serial.print("Pin: ");
      Serial.print(i);
      Serial.print(", Valore: HIGH\n");
    }
    else{
      digitalWrite(selectPins_1[i], LOW);
      Serial.print("Pin: ");
      Serial.print(i);
      Serial.print(", Valore: LOW\n");
    }
  }
}
void selectMuxPin_2(byte pin)
{
  if (pin > 15) return; // Exit if pin is out of scope
  for (int i=0; i<4; i++)
  {
    if (pin & (1<<i)){
      digitalWrite(selectPins_2[i], HIGH);
      Serial.print("Pin: ");
      Serial.print(i);
      Serial.print(", Valore: HIGH\n");
    }
    else{
      digitalWrite(selectPins_2[i], LOW);
      Serial.print("Pin: ");
      Serial.print(i);
      Serial.print(", Valore: LOW\n");
    }
  }
}
void selectMuxPin_3(byte pin)
{
  if (pin > 15) return; // Exit if pin is out of scope
  for (int i=0; i<4; i++)
  {
    if (pin & (1<<i)){
      digitalWrite(selectPins_3[i], HIGH);
      Serial.print("Pin: ");
      Serial.print(i);
      Serial.print(", Valore: HIGH\n");
    }
    else{
      digitalWrite(selectPins_3[i], LOW);
      Serial.print("Pin: ");
      Serial.print(i);
      Serial.print(", Valore: LOW\n");
    }
  }
}
void selectMuxPin_4(byte pin)
{
  if (pin > 15) return; // Exit if pin is out of scope
  for (int i=0; i<4; i++)
  {
    if (pin & (1<<i)){
      digitalWrite(selectPins_4[i], HIGH);
      Serial.print("Pin: ");
      Serial.print(i);
      Serial.print(", Valore: HIGH\n");
    }
      
    else{
      digitalWrite(selectPins_4[i], LOW);
      Serial.print("Pin: ");
      Serial.print(i);
      Serial.print(", Valore: LOW\n");
    }
  }
}
