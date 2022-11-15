#include "SPI.h"
#include <Wire.h>
#include <BH1750.h>
#include <SimpleDHT.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>



#define TFT_DC 8
#define TFT_CS 10

#define LED 7

int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);
Adafruit_MMA8451 mma = Adafruit_MMA8451();

BH1750 lightMeter;

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 

  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");
  
  mma.setRange(MMA8451_RANGE_2_G);
  
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");

  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  // For Wemos / Lolin D1 Mini Pro and the Ambient Light shield use
  // Wire.begin(D2, D1);

  // initialize digital pin LED as an output.
  pinMode(LED , OUTPUT);

  lightMeter.begin();
 
  tft.begin();
}

void loop(void) {
  LightRead();
  TempRead();
  UVRead();
  SlopeRead();

  // DHT11 sampling rate is 1HZ.
  delay(500);
  LCDWrite();
}

unsigned long SlopeRead() {
  // Read the 'raw' data in 14-bit counts
  mma.read();
  tft.print("X:\t"); tft.print(mma.x); 
  tft.print("\tY:\t"); tft.print(mma.y); 
  tft.print("\tZ:\t"); tft.print(mma.z); 
  tft.println();

  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  tft.print("X: \t"); tft.print(event.acceleration.x); tft.print("\t");
  tft.print("Y: \t"); tft.print(event.acceleration.y); tft.print("\t");
  tft.print("Z: \t"); tft.print(event.acceleration.z); tft.print("\t");
  tft.println("m/s^2 ");
  
  /* Get the orientation of the sensor */
  uint8_t o = mma.getOrientation();
  
  switch (o) {
    case MMA8451_PL_PUF: 
      Serial.println("Portrait Up Front");
      break;
    case MMA8451_PL_PUB: 
      Serial.println("Portrait Up Back");
      break;    
    case MMA8451_PL_PDF: 
      Serial.println("Portrait Down Front");
      break;
    case MMA8451_PL_PDB: 
      Serial.println("Portrait Down Back");
      break;
    case MMA8451_PL_LRF: 
      Serial.println("Landscape Right Front");
      break;
    case MMA8451_PL_LRB: 
      Serial.println("Landscape Right Back");
      break;
    case MMA8451_PL_LLF: 
      Serial.println("Landscape Left Front");
      break;
    case MMA8451_PL_LLB: 
      Serial.println("Landscape Left Back");
      break;
    }
  Serial.println();
  
}

unsigned long UVRead() {
  float sensorVoltage; 
  float sensorValue;
 
  sensorValue = analogRead(36);
  sensorVoltage = sensorValue/1024*5.0;
  Serial.print("sensor reading = ");
  Serial.print(sensorValue);
  Serial.println("");
  Serial.print("sensor voltage = ");
  Serial.print(sensorVoltage);
  Serial.println(" V");
  delay(1000);
}

unsigned long LCDWrite() {
  tft.fillScreen(ILI9341_BLACK);   // Clears screen.
  unsigned long start = micros();
  tft.setCursor(0, 0);               // Returns display to orgin. 
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);   // Changes text color to RED!! and changes text size.
  tft.println();
  
  return micros() - start;
}

unsigned long TempRead() {
  // read without samples.
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
    return;
  }

  if(((((int)temperature * 9) / 5) + (32)) > 80) {
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.println(" High Temp!! ");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);
  }
  
  tft.print((((int)temperature * 9) / 5) + (32)); tft.print(" *F, "); 
  tft.print((int)humidity); tft.println(" H");
}

unsigned long LightRead() {
  float lux = lightMeter.readLightLevel();
  tft.print("Light: ");
  tft.print(lux);
  tft.println(" lx");
  tft.println();

  if(lux < 180) {
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.println("NIGHT");
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);
    digitalWrite(LED , HIGH);//turn the LED On by making the voltage HIGH
    tft.println();
  }
  else {
    digitalWrite(LED , LOW);// turn the LED Off by making the voltage LOW
  }
}

