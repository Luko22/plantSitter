#include <Wire.h>
#include <time.h>
#include <ArduinoJson.h>
#include <credentials.h>
#include "utils.h"

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

// Time server
const char* ntpServer = "pool.ntp.org";

bool timeInitialized = false;
unsigned long lastWatered = 0;

// Button handling variables
const int waterButton = 4;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);

  pinMode(sensorPower1, OUTPUT);
  digitalWrite(sensorPower1, LOW);
  pinMode(sensorPower2, OUTPUT);
  digitalWrite(sensorPower2, LOW);
  pinMode(sensorPower3, OUTPUT);
  digitalWrite(sensorPower3, LOW);
  pinMode(waterButton, INPUT_PULLUP);

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW); // Ensure LED is off initially

  connectAWS();

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
}

void loop() {
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
  // Publish moisture data to AWS IoT
  publishMoistureData(plantName1, moisture1, status1);
  publishMoistureData(plantName2, moisture2, status2);
  publishMoistureData(plantName3, moisture3, status3);

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

  client.loop();
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

void publishMoistureData(String plantName, int moisture, String status) {
  StaticJsonDocument<200> doc;
  doc["plant"] = plantName;
  doc["moisture"] = moisture;
  doc["status"] = status;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println("Published moisture data: " + String(jsonBuffer));
}
