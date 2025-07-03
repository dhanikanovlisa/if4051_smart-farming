#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include <BH1750.h>
#include "time.h"

#define DHTPIN 5
#define DHTTYPE DHT22
#define SOIL_MOISTURE_PIN 32
#define LED_PIN 2

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

const char *ssid = "LANTAI 3";
const char *password = "lantaitiga";
const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Time
const char* ntpServer = "pool.ntp.org";

void setup_wifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    setup_wifi();
    if (client.connect("ESP32_sensor")) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" - trying again in 2 seconds");
      delay(2000);
    }
  }
}

int getSoilMoisture() {
  int rawValue = analogRead(SOIL_MOISTURE_PIN);
  int moisturePercent = map(rawValue, 4095, 0, 0, 100);
  Serial.print("Soil Moisture Raw: ");
  Serial.print(rawValue);
  Serial.print(" -> Mapped: ");
  Serial.println(moisturePercent);
  return moisturePercent;
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);

  Serial.println("Initializing sensors...");
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  dht.begin();
  Wire.begin();
  lightMeter.begin();
  configTime(21600, 3600, ntpServer);
  Serial.println("Setup complete.");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float lux = lightMeter.readLightLevel();
  int moisture = getSoilMoisture();

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Lux: ");
  Serial.println(lux);

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // LED control
  if (lux < 20) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Ambient light low: LED ON");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Ambient light sufficient: LED OFF");
  }

  String payload = "{\"sensor_id\":\"node_1\",\"temperature\":" + String(temperature, 2) +
                   ",\"humidity\":" + String(humidity, 2) +
                   ",\"moisture_percent\":" + String(moisture) +
                   ",\"lux\":" + String(lux, 2) +
                   ",\"timestamp\":\"" + getFormattedTime() + "\"}";

  if (client.publish("esp32/smartfarming/sensor", payload.c_str())) {
    Serial.println("MQTT publish success:");
    Serial.println(payload);
  } else {
    Serial.println("MQTT publish failed");
  }

  delay(3000);
}
