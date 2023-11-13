
#define SIZE (8)

int vect[SIZE];


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("Vettore: ");
  for(int i=0; i<SIZE; i++){
    vect[i]=i;
    Serial.print(vect[i]);
    
  }
  Serial.print("\n");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("ADJACENT INJECTION PROTOCOL: \n ");
  for(int i=0;i<SIZE;i++){
    Serial.print("Current MUX 1: ");
    Serial.print(i);
    Serial.print(" Current MUX 2: ");
    Serial.print(((i+1)%SIZE));
    Serial.print("\nMeasure differential voltage pairs:\n");
    int x = i+2;
    for(int n=0; n<SIZE-3; n++){
      Serial.print("\nDifferential pair number");
      Serial.print(n);
      Serial.print("\n");
      Serial.print("Voltage MUX 3: ");
      Serial.print(x%SIZE);
      Serial.print(" Voltage MUX 4: ");
      Serial.print(((x+1)%SIZE));
      x = x+1;
      //frequencySweep();
      delay(1000);
    }
    Serial.print("\n\nEND!!!\n\n\n\n");
    delay(5000);
    
  }



  delay(1000);
  
}
