#include <Arduino_BuiltIn.h>
#include "certs.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Incoming message on topic: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload, length);
  const char* ledState = doc["led"]; // expecting "on" or "off"
  
  if (strcmp(ledState, "on") == 0) {
    digitalWrite(2, HIGH);  // Turn the LED on
    Serial.println("LED turned ON");
  } else if (strcmp(ledState, "off") == 0) {
    digitalWrite(2, LOW);   // Turn the LED off
    Serial.println("LED turned OFF");
  } else {
    Serial.println("Unknown command");
  }
}

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("Subscribed to topic: " + String(AWS_IOT_SUBSCRIBE_TOPIC));
 
  Serial.println("AWS IoT Connected!");
}

void publishMessage(int metricsValue) {
  StaticJsonDocument<200> doc;
  doc["metrics"] = metricsValue;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println("Published message: " + String(jsonBuffer));
}

