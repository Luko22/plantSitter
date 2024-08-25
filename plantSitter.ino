#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//soil moisture
/* Change these values based on your calibration values */
#define soilWet 500   // Define max value we consider soil 'wet'
#define soilDry 750   // Define min value we consider soil 'dry'

// Moisture Sensor pins
#define sensorPower 3
// #define sensorPin 4
#define sensorPin A0

//CHANGE THIS TO YOUR PLANT'S NAME
#define plantName "Basilson"

void setup() {
  Serial.begin(9600);

	pinMode(sensorPower, OUTPUT);
  pinMode(sensorPin, INPUT);
	// Initially keep the sensor OFF
	digitalWrite(sensorPower, LOW);
	
  //int SSD1306
	if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(50);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.print("Hello, I'm ");
  display.println(plantName);
  display.display(); 
  delay(2000);

}

void loop() {
	//get the reading from the function below and print it
	int moisture = readSensor();
	Serial.print("Current moisture: ");
	Serial.println(moisture);

  String status;
	// Determine status of our soil
	if (moisture < soilWet) {
    status = "I'm Hydrated"; 
		Serial.println(status);

	} else if (moisture >= soilWet && moisture < soilDry) {
    status = "I'm good"; 
		Serial.println(status);
	} else {
		status = "I'm thursty"; 
		Serial.println(status);
	}
	

	// delay(5000);	// Take readings 3 times day, every 8 hours
	// delay(1000);	// Take a reading every second for testing
					// Normally you should take reading perhaps once or twice a day
  // Serial.println("last time watered: ");
  // Serial.println(seconds ago);
  // Serial.println("minutes ago");
  // Serial.println("hours ago");
  // Serial.println("days ago");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 10);
  display.print("Name: ");
  display.println(plantName);

  display.setCursor(0, 20);
  display.print("Current moisture:");
  display.println(moisture);

  display.setCursor(0, 30);
  display.print("Last watered: ");
  display.print("X");
  display.println("d");

  display.setCursor(0, 49);
  display.print("Status: ");
  display.print(status);

  display.display(); 

  delay(5000);
}

//  This function returns the analog soil moisture measurement
int readSensor() {
	digitalWrite(sensorPower, HIGH);	// Turn the sensor ON
	delay(10);							// Allow power to settle
	int val = analogRead(sensorPin);	// Read the analog value form sensor
	digitalWrite(sensorPower, LOW);		// Turn the sensor OFF
	return val;							// Return analog moisture value
}
