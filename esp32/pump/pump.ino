#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "LANTAI 3";
const char *password = "lantaitiga";
const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

const int pumpPin = 26;
bool pumpOn = false;
unsigned long pumpStartTime = 0;
const unsigned long pumpDuration = 5000;

WiFiClient espClient;
PubSubClient client(espClient);

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
    if (client.connect("ESP32_pump")) {
      Serial.println("connected to MQTT broker");
      client.subscribe("esp32/smartfarming/sensor");
      Serial.println("Subscribed to topic: esp32/smartfarming/sensor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" - trying again in 2 seconds");
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.print("MQTT message received on topic [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  int moistureIndex = msg.indexOf("moisture_percent");
  if (moistureIndex != -1) {
    int colonIndex = msg.indexOf(":", moistureIndex);
    int commaIndex = msg.indexOf(",", colonIndex);
    int endIndex = (commaIndex != -1) ? commaIndex : msg.indexOf("}", colonIndex);
    String value = msg.substring(colonIndex + 1, endIndex);
    int moisture = value.toInt();

    Serial.print("Parsed moisture percent: ");
    Serial.println(moisture);

    if (moisture < 20 && !pumpOn) {
      Serial.println("Moisture is low. Turning ON pump.");
      digitalWrite(pumpPin, LOW);  // Turn pump ON (LOW = ON for some relay modules)
      pumpStartTime = millis();
      pumpOn = true;
    } else {
      Serial.println("Moisture OK or pump already running.");
    }
  } else {
    Serial.println("moisture_percent not found in message.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, HIGH); // OFF initially (depends on your relay module logic)
  Serial.println("Pump initialized (OFF)");
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  if (pumpOn && millis() - pumpStartTime >= pumpDuration) {
    Serial.println("Pump ON duration complete. Turning OFF pump.");
    digitalWrite(pumpPin, HIGH); // Turn pump OFF
    pumpOn = false;
  }
}
