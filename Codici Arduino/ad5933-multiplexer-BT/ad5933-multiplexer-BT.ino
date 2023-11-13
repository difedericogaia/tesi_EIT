/*
ad5933-multiplexer
    Reads impedance values from the AD5933 over I2C and prints them BTserially.
    Protocol injection: adjacent (40 measurements) or opposite (32 measurements).
*/

#include <Wire.h>
#include "AD5933.h"
#include <SoftwareSerial.h>
SoftwareSerial BTserial(0, 1); // RX | TX


#define START_FREQ  (100000)
#define FREQ_INCR   (1000)
#define NUM_INCR    (1)
#define REF_RESIST  (550)
#define SIZE (8) //number of electrodes

int vect[SIZE];
int index;
double impedance_measurement_adj[SIZE*10];
double impedance_measurement_opp[SIZE*8];
double gain[NUM_INCR+1];
int phase[NUM_INCR+1];

const int selectPins_1[3] = {2,3,4}; // S0~2, S1~3, S2~4
const int selectPins_2[3] = {5,6,7}; // S0~5, S1~6, S2~7
const int selectPins_3[3] = {8,9,10}; // S0~8, S1~9, S2~10
const int selectPins_4[3] = {11,12,13}; // S0~11, S1~12, S2~13

void setup(void)
{
  // Begin I2C
  Wire.begin();
  BTserial.begin(9600);
  delay(10);
  BTserial.println("ready BT...");
  
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

  //EN BT
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  
  BTserial.println("ADJACENT PROTOCOL: 'a' ");
  BTserial.println("OPPOSITE PROTOCOL: 'b' ");
  
  // Begin BTserial at 9600 baud for output

  BTserial.println("AD5933 Test Started!");

  //select MUX for calibration: current injection 0-4, voltage measurement 1-5
  selectMuxPin_1(0);
  selectMuxPin_2(4);
  selectMuxPin_3(1);
  selectMuxPin_4(5);
  delay(1);
  
  // Perform initial configuration. Fail if any one of these fail.
  if (!(AD5933::reset() &&
        AD5933::setInternalClock(true) &&
        AD5933::setStartFrequency(START_FREQ) &&
        AD5933::setIncrementFrequency(FREQ_INCR) &&
        AD5933::setNumberIncrements(NUM_INCR) &&
        AD5933::setPGAGain(PGA_GAIN_X5)))
        {
            BTserial.println("FAILED in initialization!");
            while (true) ;
        }

  // Perform calibration sweep
  if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR+1))
    BTserial.println("Calibrated!");
  else
    BTserial.println("Calibration failed...");
  
}

void loop(void)
{
   if (BTserial.available()) {
    switch (BTserial.read()) {
      case 'a':
        index=0;
        //BTBTserial.print("\n---ADJACENT INJECTION PROTOCOL---\n ");
        for(int i=0;i<SIZE;i++){
          selectMuxPin_1(i);
          //BTBTserial.print("Current MUX 1:  ");
          //BTBTserial.print(i);
          selectMuxPin_2(((i+1)%SIZE));
          //BTBTserial.print("  Current MUX 2:  ");
          //BTBTserial.print(((i+1)%SIZE));
          //BTBTserial.print("\nMeasure differential voltage pairs: \n");
          delay(10);
          int x = i+2;
          for(int n=0; n<SIZE-3; n++){
            
            //BTBTserial.print("\nDifferential pair number: ");
            //BTBTserial.print(n);
            //BTBTserial.print("\n");
            selectMuxPin_3(x%SIZE);
            //BTBTserial.print("Voltage MUX 3:  ");
            //BTBTserial.print(x%SIZE);
            //BTBTserial.print("  Voltage MUX 4:  ");
            selectMuxPin_4((x+1)%SIZE);
            //BTBTserial.print(((x+1)%SIZE));
            x = x+1;
            delay(10);
            frequencySweepAdj();
            delay(10);
          }          
        }
        //BTBTserial.print("\n\nEND!!!\n\n\n\n");
        //BTBTserial.print("Numero dati acquisiti: ");
        //BTBTserial.println(sizeof(impedance_measurement_adj)/4);
        for (int i=0; i<SIZE*10; i++){
          if(i%2==0){
           BTserial.print(impedance_measurement_adj[i]);
           BTserial.print(",");
          }
        }
        BTserial.print("\n");
        break;
      case 'b':
        index=0;
        //BTBTserial.print("\n---OPPOSITE INJECTION PROTOCOL:--- ");
        for(int i=0;i<SIZE;i++){
          selectMuxPin_1(i);
          //BTBTserial.print("Current MUX 1:  ");
          //BTBTserial.print(i);
          selectMuxPin_2(((i+4)%SIZE));
          //BTBTserial.print("  Current MUX 2:  ");
          //BTBTserial.print(((i+1)%SIZE));
          //BTBTserial.print("\nMeasure differential voltage pairs: \n");
          delay(0.1);
          int x = i+1;
          for(int n=0; n<2; n++){
            //BTBTserial.print("\nDifferential pair number: ");
            //BTBTserial.print(n);
            //BTBTserial.print("\n");
            selectMuxPin_3(x%SIZE);
            //BTBTserial.print("Voltage MUX 3:  ");
            //BTBTserial.print(x%SIZE);
            //BTBTserial.print("  Voltage MUX 4:  ");
            selectMuxPin_4((x+1)%SIZE);
            //BTBTserial.print(((x+1)%SIZE));
            x = x+1;
            delay(1);
            frequencySweepOpp();
            delay(1);
          }
          x=x+3;
          for(int n=0; n<2; n++){
            selectMuxPin_3(x%SIZE);
            selectMuxPin_4((x+1)%SIZE);   
            x = x+1;  
            delay(1);
            frequencySweepOpp();
            delay(1);    
          }
        }
        //BTBTserial.print("\n\nEND!!!\n\n\n\n");
        //BTBTserial.print("\nNumero dati acquisiti: ");
        //BTBTserial.println(sizeof(impedance_measurement_opp)/4);
        for (int i=0; i<SIZE*8; i++){
          if(i%2==0){
           BTserial.print(impedance_measurement_opp[i]);
           BTserial.print(",");
          }
        }
        BTserial.print("\n");
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
        
        /*BTBTserial.print(cfreq);
        BTBTserial.print(": R=");
        BTserial.print(real[i]);
        BTserial.print("/I=");
        BTserial.print(imag[i]);*/

        // Compute impedance
        double magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
        double impedance = 1/(magnitude*gain[i]);
        //BTserial.print("\n  |Z|=");
        //BTserial.println(impedance);
        impedance_measurement_adj[index]=impedance;
        index = index+1;
      }
      //BTserial.println("Frequency sweep complete!");
    } else {
      BTserial.println("Frequency sweep failed...");
    }
}

void frequencySweepOpp() {
    // Create arrays to hold the data
    int real[NUM_INCR+1], imag[NUM_INCR+1];

    // Perform the frequency sweep
    if (AD5933::frequencySweep(real, imag, NUM_INCR+1)) {
      // Print the frequency data
      int cfreq = START_FREQ/1000;
      for (int i = 0; i < NUM_INCR+1; i++, cfreq += FREQ_INCR/1000) {
        // Print raw frequency data
        /*BTserial.print(cfreq);
        BTserial.print(": R=");
        BTserial.print(real[i]);
        BTserial.print("/I=");
        BTserial.print(imag[i]);
*/
        // Compute impedance
        double magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
        double impedance = 1/(magnitude*gain[i]);
        /*BTserial.print("\n  |Z|=");
        BTserial.println(impedance);*/
        impedance_measurement_opp[index]=impedance;
        index = index+1;
      }
      //BTserial.println("Frequency sweep complete!");
    } else {
      BTserial.println("Frequency sweep failed...");
    }
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
