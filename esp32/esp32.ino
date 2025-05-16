#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
// #include <Wire.h>
// #include <BH1750.h>

#define DHTPIN 5     // Pin where the DHT22 data pin is connected
#define DHTTYPE DHT22   // DHT 22 (AM2302)
#define SOIL_MOISTURE_PIN 32
DHT dht(DHTPIN, DHTTYPE);
// BH1750 lightMeter;

// WiFi credentials
const char *ssid = "Aidha’s iPhone";
const char *password = "lisawifi";

// MQTT Broker details
const char *mqtt_server = "212.85.26.216";
const int mqtt_port = 1883;

// LED pin (added since you're using blink_led function)
const int ledPin = 2;  // Typically GPIO2 on many ESP32 boards

// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

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
  Serial.print("raw value : ");
  Serial.println(rawValue);
  return map(rawValue, 4095, 0, 0, 100); // Adjust based on your sensor's calibration
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  dht.begin();
  // Wire.begin();
  // lightMeter.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  //float lux = lightMeter.readLightLevel();
  float lux = 4.0;
  int moisturePercent = getSoilMoisture();
  Serial.print(" | Moisture: ");
  Serial.println(moisturePercent);
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Build JSON-style payload
  String payload = "{\"sensor_id\": \"node_1\", \"temperature\":" + String(temperature, 2) + 
    ", \"humidity\":" + String(humidity, 2) + 
    ", \"moisture_percent\":" + String(moisturePercent) + 
    ", \"lux\":" + String(lux, 2) + 
    ", \"timestamp\": \"2025-05-07T13:00:00Z\"}";

  // Publish to MQTT topic
  if (client.publish("esp32/sensor1", payload.c_str())) {
    Serial.println("Published: " + payload);
  } else {
    Serial.println("Publish failed");
  }

  delay(3000);  // Wait 3 seconds between publishes
}
