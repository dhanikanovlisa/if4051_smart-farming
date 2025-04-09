import os
from dotenv import load_dotenv
load_dotenv()

MQTT_BROKER_IP = os.getenv("MQTT_BROKER_IP", "127.0.0.1")
MQTT_BROKER_PORT = int(os.getenv("MQTT_BROKER_PORT", 1883))
MQTT_TOPIC = "esp32/#"
DEBUG = os.getenv("DEBUG", "False").lower() == "true"