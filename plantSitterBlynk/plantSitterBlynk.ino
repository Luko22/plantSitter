#define BLYNK_PRINT Serial
/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_NAME "plantSitter"
#define BLYNK_AUTH_TOKEN "YBdVFppzLLNMLMEfqKVgnI8N56qReS0l"
#define BLYNK_TEMPLATE_ID "TMPL4Ino2K8h1"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>
#include <credentials.h>
#include <BlynkSimpleEsp32.h>
#include "esp_http_server.h"

// Screen dimensions and I2C address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C  // Change if needed

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



// Wi-Fi credentials
const char* ssid = mySSIDLap;
const char* password = myPASSWORDLap;
// Time server
const char* ntpServer = "pool.ntp.org";

//button global variables
int waterButton;
int wButtonState = HIGH;

// Soil moisture levels
#define soilWet 2000
#define soilDry 3500

// Pin definitions
#define sensorPower 33
#define sensorPin 36

#define plantName "Basilson"

// Timekeeping variables
unsigned long lastWatered = 0;

bool timeInitialized = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = HIGH;

BLYNK_WRITE(V1) {
  waterButton = param.asInt();
}

void setup() {
  Serial.begin(115200);

  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  // Set up current local time
  configTime(3600, 3600, ntpServer);
  delay(2000); // Wait for time to be synced

  // Wait for time to be set
    int timeoutCounter = 0;
    while (!timeInitialized && timeoutCounter < 10) {
      time_t now;
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        timeInitialized = true;
        time(&now);
        lastWatered = now - (2 * 365 * 24 * 60 * 60); // Initialize to two years ago, should only be changed by pressing the water button
        Serial.println("Time initialized successfully");
      } else {
        delay(1000);
        timeoutCounter++;
      }
    } 
    if (!timeInitialized) {
      Serial.println("Failed to initialize time");
      } else {}

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println();

  // Display initial message
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 30);
  display.print("Hello, I'm ");
  display.println(plantName);
  display.display();

  delay(2000);
}//end of setup

void loop() {
  Blynk.run();

  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  int moisture = readSensor();
  Blynk.virtualWrite(V0, moisture);
  Serial.print("Current moisture: ");
  Serial.println(moisture);

 // Calculate days since last watering
  int daysSinceWatered = (now - lastWatered) / (60 * 60 * 24);
  
  String status;
  if (moisture < soilWet) {
    status = "I'm Hydrated"; 
  } else if (moisture >= soilWet && moisture < soilDry) {
    status = "I'm good"; 
  } else {
    status = "I'm thirsty"; 
  }

  int reading = waterButton;

  // Check for button press with debouncing
  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // reset the debouncing timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed and the delay has passed
    if (reading != wButtonState) {
      wButtonState = reading;

      // Only consider the press if the button is LOW (pressed)
      if (wButtonState == LOW) {
        Serial.println("");
        Serial.println("Watered the plant");
        Serial.println("");
        lastWatered = now;
      }
    }
  }lastButtonState = reading;

  // Serial.print(timeinfo.tm_mday, DEC);
  // Serial.print("/");
  // Serial.print(timeinfo.tm_mon + 1, DEC); // Months are 0-based
  // Serial.print("/");
  // Serial.print(timeinfo.tm_year + 1900, DEC);  

  // Serial.print("  ");
  // Serial.print(timeinfo.tm_hour, DEC);
  // Serial.print(":");
  // Serial.println(timeinfo.tm_min, DEC);

  // Serial.print("Last watered: ");
  // Serial.print(daysSinceWatered, DEC);
  // Serial.println("d");

/////////////////////////////////
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Display date and time
  display.setCursor(0, 0);
  display.print(timeinfo.tm_mday, DEC);
  display.print("/");
  display.print(timeinfo.tm_mon + 1, DEC); // Months are 0-based
  display.print("/");
  display.print(timeinfo.tm_year + 1900, DEC);

  display.setCursor(95, 0);
  display.print(timeinfo.tm_hour, DEC);
  display.print(":");
  display.print(timeinfo.tm_min, DEC);

  // Display plant information
  display.setCursor(40, 10);
  display.print("Blynk App");

  display.setCursor(0, 20);
  display.print("Name: ");
  display.println(plantName);

  display.setCursor(0, 30);
  display.print("Current moisture:");
  display.println(moisture);

  display.setCursor(0, 40);
  display.print("Last watered: ");
  display.print(daysSinceWatered, DEC);
  display.print("d");

  display.setCursor(0, 60);
  display.print("Status: ");
  display.println(status);
  display.display();

  delay(1000);  // Display for 10 seconds
  Serial.println("");
  Serial.println("");
}//end of loop

int readSensor() {
  digitalWrite(sensorPower, HIGH);
  delay(10);
  int val = analogRead(sensorPin);
  digitalWrite(sensorPower, LOW);

  return val;
}
