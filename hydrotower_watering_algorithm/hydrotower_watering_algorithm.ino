// Ultrasonic
#include "Ultrasonic.h"

// setup OLED Display
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// I2C High Accuracy Temp&Humi
#include <Wire.h>
#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();

#define SCREEN_WIDTH 128     // OLED display width
#define SCREEN_HEIGHT 64     // OLED display height
#define OLED_RESET -1        // Reset pin number, -1 means Arduino reset pin
#define SCREEN_ADDRESS 0x3C  // Address may be either 3C or 3d
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Ultrasonic
Ultrasonic ultrasonic(12);

// Moisture Sensor
int MoistureSensorPin = A6;

// Relay
const int relayPin = 11;

// Timer for relay + Countdown
unsigned long endTime;
unsigned long elapsedTime;
unsigned long timeLastRun = 0;
unsigned long timeSinceLastRun;
bool timerStarted = false;




void setup() {
  // put your setup code here, to run once:

  // Logging
  Serial.begin(9600);

  // OLED
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  // Relay
  pinMode(relayPin, OUTPUT);
  // I2C High Accuracy Temp&Humi
  if (!sht31.begin(0x45)) {
    Serial.println("SHT31 not found");
  }

  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();  // Clears display buffer
  display.setCursor(0, 0);  // Start printing at top-left corner
  // Reading sensor values
  int moistureValue = readMoistureSensor();
  int ultraSonicValue = readUltrasonicSensor();
  float hATemperature = readHighAccuracyTemperature();
  float hAHumidity = readHighAccuracyHumidity();

  if (ultraSonicValue < 10) {
    if (timerStarted) {
      stopWateringProgram();
    }
    display.print("Water Tank is empty, please fill it !");

  } else {
    display.print("Mst:" + String(moistureValue));
    display.print(" | US:" + String(ultraSonicValue));
    display.println("");  // empty line

    display.println("HA Temp:" + String(hATemperature));
    display.println("HA Hum:" + String(hAHumidity));
    display.println("");  // empty line

    if (!timerStarted) {
      determineWateringProgram(moistureValue, ultraSonicValue, hATemperature, hAHumidity);
    }
    display.println(relayTimer());
  }

  display.display();  //displays the text on screen
  delay(1000);
}

int readMoistureSensor() {
  return analogRead(MoistureSensorPin);
}

int readUltrasonicSensor() {
  return ultrasonic.MeasureInCentimeters();
}

float readHighAccuracyTemperature() {
  return sht31.readTemperature();
}

float readHighAccuracyHumidity() {
  return sht31.readHumidity();
}

void relayOn() {
  digitalWrite(relayPin, HIGH);
}

void relayOff() {
  digitalWrite(relayPin, LOW);
}

void relayOnForXTime(int delaySeconds) {
  if (!timerStarted) {
    endTime = millis() + (delaySeconds * 1000UL);
    timerStarted = true;
    relayOn();
  }
}

String relayTimer() {

  if (timerStarted) {
    elapsedTime = (endTime - millis()) / 1000UL;

    if (elapsedTime < endTime) {
      return "relayOnFor: " + String(elapsedTime) + "s";

    } else {
      timerStarted = false;
      relayOff();
      timeLastRun = millis();
    }
  }

  if (!timerStarted) {
    if (timeLastRun == 0) {
      return "noRunYet";
    }

    timeSinceLastRun = (millis() - timeLastRun) / 1000UL;
    return "lastRunSin.: " + String(timeSinceLastRun) + "s";
  }
}

void stopWateringProgram() {
    relayOff();
    timerStarted = false;
    timeLastRun = millis();
}

// here you can add further sensor values
void determineWateringProgram(int moistureValue, int ultraSonicValue, float hATemperature, float hAHumidity) {

  if (moistureValue >= 10 && moistureValue <= 30 &&
   ultraSonicValue >= 500 && ultraSonicValue <= 560 &&
    hATemperature >= 20 && hATemperature <= 40 &&
     hAHumidity >= 20 && hAHumidity <= 40) {
    relayOnForXTime(5);
  }

  else if (moistureValue >= 31 && moistureValue <= 120 &&
   ultraSonicValue >= 500 && ultraSonicValue <= 560 &&
    hATemperature >= 20 && hATemperature <= 40 &&
     hAHumidity >= 20 && hAHumidity <= 40) {
    relayOnForXTime(15);
  }

  else {
    Serial.println("No Condition applicable");
  }
}
