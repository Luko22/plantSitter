#include <Arduino.h>

#define BLYNK_PRINT Serial
/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_NAME "plantSitter"
#define BLYNK_AUTH_TOKEN "YBdVFppzLLNMLMEfqKVgnI8N56qReS0l"
#define BLYNK_TEMPLATE_ID "TMPL4Ino2K8h1"

#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>
// #include <credentials.h>
#include <BlynkSimpleEsp32.h>
#include "esp_http_server.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Soil moisture levels
#define soilWet 1000
#define soilDry 2700

// Pin definitions
#define plantName1 "Basilson"
#define sensorPin1 36
#define sensorPower1 25

#define plantName2 "Calathius"
#define sensorPin2 34
#define sensorPower2 26

#define plantName3 "Monsteria"
#define sensorPin3 32
#define sensorPower3 27

// Wi-Fi credentials
const char* ssid = "ROGspot";
const char* password = "Kltown5.";

// Time server
const char* ntpServer = "pool.ntp.org";

bool timeInitialized = false;
unsigned long lastWatered = 0;

// Button handling variables
const int waterButton = 15;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = HIGH;
int blynkWaterButtonState = HIGH; // New variable to store Blynk button state



int readSensor(int plantPin, int power);
String getStatus(int moisture);

void setup() {
  Serial.begin(115200);
  
  pinMode(waterButton, INPUT_PULLUP);
  pinMode(sensorPower1, OUTPUT);
  digitalWrite(sensorPower1, LOW);
  pinMode(sensorPower2, OUTPUT);
  digitalWrite(sensorPower2, LOW);
  pinMode(sensorPower3, OUTPUT);
  digitalWrite(sensorPower3, LOW);
  pinMode(waterButton, INPUT_PULLUP);

  // pinMode(2, OUTPUT);
  // digitalWrite(2, LOW); // Ensure LED is off initially

  // Set up time
  configTime(3600, 3600, ntpServer);
  delay(2000);

  // Wait for time to be set
  int timeoutCounter = 0;
  while (!timeInitialized && timeoutCounter < 10) {
    time_t now;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      timeInitialized = true;
      time(&now);
      lastWatered = now - (2 * 365 * 24 * 60 * 60);
      Serial.println("Time initialized successfully");
    } else {
      delay(1000);
      timeoutCounter++;
    }
  }

  if (!timeInitialized) {
    Serial.println("Failed to initialize time");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println();

  delay(2000);
}

void loop() {
  Blynk.run();

  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  int moisture1 = readSensor(sensorPin1, sensorPower1); 
  int moisture2 = readSensor(sensorPin2, sensorPower2);
  int moisture3 = readSensor(sensorPin3, sensorPower3);

  String status1 = getStatus(moisture1);
  String status2 = getStatus(moisture2);
  String status3 = getStatus(moisture3);

  Serial.print("Plant 1: "); Serial.print(moisture1); Serial.print(" | ");
  Serial.print("Plant 2: "); Serial.print(moisture2); Serial.print(" | ");
  Serial.print("Plant 3: "); Serial.print(moisture3); Serial.print(" | ");
  Serial.println();

  Blynk.virtualWrite(V0, moisture1);
  Blynk.virtualWrite(V1, moisture2);
  Blynk.virtualWrite(V2, moisture3);

  Blynk.virtualWrite(V3, status1);
  Blynk.virtualWrite(V4, status2);
  Blynk.virtualWrite(V5, status3);

  int reading = digitalRead(waterButton);
  // Check for button press with debouncing
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW) {
      Serial.println("\nWatering plant");
      delay(2000);
    }
  }

  lastButtonState = reading;

  int daysSinceWatered = (now - lastWatered) / (60 * 60 * 24);

  Serial.print(timeinfo.tm_mday, DEC);
  Serial.print("/");
  Serial.print(timeinfo.tm_mon + 1, DEC);
  Serial.print("/");
  Serial.print(timeinfo.tm_year + 1900, DEC);  
  Serial.print("  ");
  Serial.print(timeinfo.tm_hour, DEC);
  Serial.print(":");
  Serial.println(timeinfo.tm_min, DEC);

  Serial.print("Last watered: ");
  Serial.print(daysSinceWatered, DEC);
  Serial.println("d");

  delay(10000);

  Serial.println("");
  Serial.println("");
}

int readSensor(int plantPin, int power) {
  digitalWrite(power, HIGH);
  delay(10);
  int val = analogRead(plantPin);
  digitalWrite(power, LOW);
  return val;
}

String getStatus(int moisture) {
  if (moisture < soilWet) {
    return "I'm Hydrated";
  } else if (moisture >= soilWet && moisture < soilDry) {
    return "I'm good";
  } else {
    return "I'm thirsty";
  }
}