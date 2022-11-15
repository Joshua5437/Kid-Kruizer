#include "SPI.h"
#include <Wire.h>
#include <BH1750.h>
#include <SimpleDHT.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>




#define TFT_DC 8
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_CS 10

#define LED 7

int x, y, z;
double roll = 0.00, pitch = 0.00;

int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);
Adafruit_MMA8451 mma = Adafruit_MMA8451();

BH1750 lightMeter;

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  Serial.println(F("Initialized"));

  tft.init(240, 320);

  uint16_t time = millis();
  tft.fillScreen(ST77XX_WHITE);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  // large block of text
  tft.fillScreen(ST77XX_WHITE);

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
}

void loop(void) {
  LightRead();
  TempRead();
  SlopeRead();
  UVRead();
  

  // DHT11 sampling rate is 1HZ.
  delay(1500);
  LCDWrite();
}

unsigned long SlopeRead() {
  mma.read();

  double x_Buff = mma.x;
  double y_Buff = mma.y;
  double z_Buff = mma.z;
  roll = atan2(y_Buff, z_Buff) * 57.3;
  pitch = atan2((- x_Buff), sqrt(y_Buff * y_Buff + z_Buff * z_Buff)) * 57.3;

  if(roll > 15) { 
    tft.setTextColor(ST77XX_RED);
    tft.println();
    tft.println("Downhill Slope Detected!! "); 
    tft.setTextColor(ST77XX_BLACK);
    tft.setTextSize(2);
  }
  else if (roll < -15) {
    tft.setTextColor(ST77XX_RED);
    tft.println();
    tft.println("Uphill Slope Detected!! "); 
    tft.setTextColor(ST77XX_BLACK);
    tft.setTextSize(2);
  }

  Serial.println(roll);
  Serial.println(pitch);
  Serial.println();
}

unsigned long UVRead() {
  float sensorVoltage; 
  float sensorValue;
 
  sensorValue = analogRead(36);
  sensorVoltage = sensorValue/1024*5.0;
  tft.println();
  tft.print("UV Index = ");
  tft.print(sensorValue);
  tft.println();
  tft.print("Voltage = ");
  tft.print(sensorVoltage);
  tft.println(" V");
}

unsigned long LCDWrite() {
  tft.fillScreen(ST77XX_WHITE); // Clears screen.
  unsigned long start = micros();
  tft.setCursor(0, 0);               // Returns display to orgin. 
  tft.setTextColor(ST77XX_BLACK); tft.setTextSize(2);   // Changes text color to RED!! and changes text size.
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
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(3);
    tft.println(" High Temp!! ");
    tft.setTextColor(ST77XX_BLACK);
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
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(3);
    tft.println("NIGHT");
    tft.setTextColor(ST77XX_BLACK);
    tft.setTextSize(2);
    digitalWrite(LED , HIGH);//turn the LED On by making the voltage HIGH
    tft.println();
  }
  else {
    digitalWrite(LED , LOW);// turn the LED Off by making the voltage LOW
  }
}