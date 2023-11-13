/*
ad5933-test
    Reads impedance values from the AD5933 over I2C and prints them serially.
*/

#include <Wire.h>
#include "AD5933.h"


#define START_FREQ  (80000)
#define FREQ_INCR   (1000)
#define NUM_INCR    (40)
#define REF_RESIST  (10000)

typedef union {
  float floatingPoint;
  byte binary[4];
} binaryFloat;

double gain[NUM_INCR+1];
double data[NUM_INCR+1];
binaryFloat impedance[NUM_INCR+1];
int phase[NUM_INCR+1];



void setup(void)
{
  // Begin I2C
  Wire.begin();

  // Begin serial at 9600 baud for output
  Serial.begin(9600);
  //Serial.println("AD5933 Test Started!");
  pinMode(LED_BUILTIN, OUTPUT);

  // Perform initial configuration. Fail if any one of these fail.
  if (!(AD5933::reset() &&
        AD5933::setInternalClock(true) &&
        AD5933::setStartFrequency(START_FREQ) &&
        AD5933::setIncrementFrequency(FREQ_INCR) &&
        AD5933::setNumberIncrements(NUM_INCR) &&
        AD5933::setPGAGain(PGA_GAIN_X1)))
        {
            //Serial.println("FAILED in initialization!");
            while (true) ;
        }

  // Perform calibration sweep
  if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR+1))
     for(int i=0; i<2;i++){
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          delay(200);                      // wait for a second
          digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
          delay(200);
        }
  else
    digitalWrite(LED_BUILTIN, LOW);
  //data[0]=0x0A;
  //data[NUM_INCR+1+1]=0x0B;
}

void loop(void)
{
   if (Serial.available()) {
    switch (Serial.read()) {
      case 'a':
        //Serial.println("Perform a EASY frequency sweep:");
        
        frequencySweepEasy();
        for(int i=0; i<5;i++){
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          delay(200);                      // wait for a second
          digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
          delay(200);
        }
        //Serial.println(data[0]);
        //for(int i=1; i<NUM_INCR+2; i++){
          //data[i]=impedance[i-1];
          //Serial.println(data[i]);
        //}
        //Serial.println(data[NUM_INCR+2+1]);
        //byte *data_byte = (byte*)data;
        //byte *data_byte = (byte*)impedance;
        //for(byte i = 0; i < sizeof(impedance); i++){
        //  Serial.write(data_byte[i]);
        //}
        //Serial.print("\nNumero dati raccolti:");
        //Serial.println(sizeof(impedance));
        //Serial.print("\nNumero bytes inviati:");
        //Serial.println(sizeof(data_byte));
        
        //delay(1000);
        
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

// Easy way to do a frequency sweep. Does an entire frequency sweep at once and
// stores the data into arrays for processing afterwards. This is easy-to-use,
// but doesn't allow you to process data in real time.
void frequencySweepEasy() {
    // Create arrays to hold the data
    int real[NUM_INCR+1], imag[NUM_INCR+1];

    // Perform the frequency sweep
    if (AD5933::frequencySweep(real, imag, NUM_INCR+1)) {
      // Print the frequency data
      byte b = 0;
      int count =0;
      int count_b =0;
      int size_imp =0;
      int cfreq = START_FREQ/1000;
      for (int i = 0; i < NUM_INCR+1; i++, cfreq += FREQ_INCR/1000) {
        // Compute impedance
        float magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
        binaryFloat impedance;
        impedance.floatingPoint= 1/(magnitude*gain[i]);
        /*Serial.print("Floating point: ");
        Serial.print(impedance.floatingPoint);
        Serial.print(" Binary: ");
        Serial.print(impedance.binary[0]);
        Serial.print(impedance.binary[1]);
        Serial.print(impedance.binary[2]);
        Serial.print(impedance.binary[3]);
        Serial.print("\n");*/
        Serial.write(impedance.binary,4);
        count +=1;        
      }
      //Serial.println("Frequency sweep complete!");
    //} else {
      //Serial.println("Frequency sweep failed...");
       /* Serial.print("\nNumero dati raccolti:");
        Serial.println(count);
        Serial.print("\nNumero bytes inviati:");
        Serial.println(count_b);
        Serial.print("\nGrandezza di 1 dato in bytes:");
        Serial.println(size_imp);*/
    }
    
}
