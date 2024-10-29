#include <HX711-multi.h>

// Pins to the load cell amp
#define CLK A0      // clock pin to the load cell amp
#define DOUT1 A1    // data pin to the 1st lca
#define DOUT2 A2    // data pin to the 2nd lca
#define DOUT3 A3    // data pin to the 3rd lca
#define DOUT4 A4    // data pin to the 4th lca
#define DOUT5 A5    // data pin to the 5th lca
#define DOUT6 2    // data pin to the 6th lca
#define DOUT7 3    // data pin to the 7th lca
#define DOUT8 4    // data pin to the 8th lca

#define BOOT_MESSAGE "MIT_ML_SCALE V0.8"

#define TARE_TIMEOUT_SECONDS 4

byte DOUTS[8] = {DOUT1, DOUT2, DOUT3, DOUT4, DOUT5, DOUT6, DOUT7, DOUT8};

#define CHANNEL_COUNT sizeof(DOUTS)/sizeof(byte)

long int results[CHANNEL_COUNT];

HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);

void setup() {
  Serial.begin(115200);
  Serial.println(BOOT_MESSAGE);
  Serial.flush();
  pinMode(11, OUTPUT);
  
  tare();
}


void tare() {
  bool tareSuccessful = false;

  unsigned long tareStartTime = millis();
  while (!tareSuccessful && millis() < (tareStartTime + TARE_TIMEOUT_SECONDS*1000)) {
    tareSuccessful = scales.tare(20, 10000);  //reject 'tare' if still ringing
  }
}

void sendRawData() {
  scales.read(results);
  for (int i=0; i < scales.get_count(); ++i) {;
    // Grams
    Serial.println(results[i] / 0.42);  
    // Serial.print( (i!=scales.get_count()-1)?"\t":"\n");
  }
  Serial.println();
  delay(1000);
}

void loop() {
  sendRawData(); //this is for sending raw data, for where everything else is done in processing

  //on serial data (any data) re-tare
  if (Serial.available() > 0) {
    while (Serial.available()) {
      Serial.read();
    }
    tare();
  }
 
}