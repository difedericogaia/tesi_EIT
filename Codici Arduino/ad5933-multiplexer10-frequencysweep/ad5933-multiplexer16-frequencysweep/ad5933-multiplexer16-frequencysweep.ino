/*
ad5933-multiplexer
    16 electrodes setup
    Reads impedance values from the AD5933 over I2C and prints them serially at the end of each cycle of injection. 
    Protocol injection: adjacent (40 measurements) or opposite (32 measurements).
*/
//provo calib 0123 invece che 0814
//ref=1000 invece che 500

#include <Wire.h>
#include "AD5933.h"

#define START_FREQ  (10000)
#define FREQ_INCR   (10000)
#define NUM_INCR    (9)
#define REF_RESIST  (1000)
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
  //0412,0123 funziona, 
  selectMuxPin_1(4);
  selectMuxPin_2(5);
  selectMuxPin_3(6);
  selectMuxPin_4(7);
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
        //index=0;
        //Serial.print("\n---ADJACENT INJECTION PROTOCOL---\n ");
        for(int i=0;i<SIZE;i++){
          selectMuxPin_1(i);
          //Serial.print("Current MUX 1:  ");
          //Serial.print(i);
          selectMuxPin_2(((i+1)%SIZE));
          //Serial.print("  Current MUX 2:  ");
          //Serial.print(((i+1)%SIZE));
          //Serial.print("\nMeasure differential voltage pairs: \n");
          delay(1);
          int x = i+2;
          for(int n=0; n<SIZE-3; n++){
            
            //Serial.print("\nDifferential pair number: ");
            //Serial.print(n);
            //Serial.print("\n");
            selectMuxPin_3(x%SIZE);
            //Serial.print("Voltage MUX 3:  ");
            //Serial.print(x%SIZE);
            //Serial.print("  Voltage MUX 4:  ");
            selectMuxPin_4((x+1)%SIZE);
            //Serial.print(((x+1)%SIZE));
            x = x+1;
            delay(1);
            frequencySweepAdj();
            
            delay(1);
          }          
        }
        //Serial.print(";\n");
        Serial.print(";\n\nEND!!!\n\n\n\n");
        //Serial.print("Numero dati acquisiti: ");
        //Serial.println(sizeof(impedance_measurement_adj)/4);
        /*for (int i=0; i<SIZE*10; i++){
          if(i%2==0){
           Serial.print(impedance_measurement_adj[i]);
           Serial.print(",");
          }
        }*/
        //Serial.println("END!");
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
          //Serial.print(impedance);     
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
