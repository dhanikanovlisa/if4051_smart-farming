#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include <BH1750.h>
#include "time.h"

#define DHTPIN 5     // Pin where the DHT22 data pin is connected
#define DHTTYPE DHT22   // DHT 22 (AM2302)
#define SOIL_MOISTURE_PIN 32
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

// WiFi credentials
const char *ssid = "bitha";
const char *password = "fqww3362";

// MQTT Broker details
const char *mqtt_server = "212.85.26.216";
const int mqtt_port = 1883;

// LED pin (added since you're using blink_led function)
const int ledPin = 2;  // Typically GPIO2 on many ESP32 boards

const int pumpPin = 26;
bool pumpOn = false;
unsigned long pumpStartTime = 0;
const unsigned long pumpDuration = 5000; // 5 seconds

// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

//Time 
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// LED blink function (added since you're using it)
void blink_led(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW);
    delay(duration);
  }
}

// MQTT callback function (added since you're using it)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      // If not connected, first connect to WiFi
      setup_wifi();
    }

    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_client1")) { 
      Serial.println("connected");
      // Subscribe to topics
      client.subscribe("rpi/broadcast");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 2 seconds");
      
      blink_led(3, 200); 
      delay(2000);
    }
  }
}

int getSoilMoisture()
{
  int rawValue = analogRead(SOIL_MOISTURE_PIN);
  return map(rawValue, 4095, 0, 0, 100); // Adjust based on your sensor's calibration
}

String getFormattedTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "";
  }
  
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
  pinMode(SOIL_MOISTURE_PIN, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  dht.begin();
  configTime(21600, 3600, "pool.ntp.org");
  Wire.begin();
  lightMeter.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float lux = lightMeter.readLightLevel();
//  float lux = random(10);
  int moisturePercent = getSoilMoisture();
  // Serial.print(" | Time :  ");
  // Serial.println(moisturePercent);
  // Serial.println(getFormattedTime());
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Build JSON-style payload
  String payload = "{\"sensor_id\": \"node_1\", \"temperature\":" + String(temperature, 2) + 
    ", \"humidity\":" + String(humidity, 2) + 
    ", \"moisture_percent\":" + String(moisturePercent) + 
    ", \"lux\":" + String(lux, 2) + 
    ", \"timestamp\": \""+getFormattedTime()+"\"}";

  if(lux<20){
    digitalWrite(ledPin, HIGH);
  }
  else{
    digitalWrite(ledPin, LOW);
  }

  if (moisturePercent < 20 && !pumpOn) {
    Serial.println("Turning on pump (non-blocking)");
    digitalWrite(pumpPin, LOW);
    pumpStartTime = millis();
    pumpOn = true;
  }
  
  // Turn off pump after pumpDuration
  if (pumpOn && millis() - pumpStartTime >= pumpDuration) {
    Serial.println("Turning off pump");
    digitalWrite(pumpPin, HIGH);
    pumpOn = false;
  }
 
  // Publish to MQTT topic
  if (client.publish("esp32/sensor1", payload.c_str())) {
    Serial.println("Published: " + payload);
  } else {
    Serial.println("Publish failed");
  }

  delay(3000);  // Wait 3 seconds between publishes
}
