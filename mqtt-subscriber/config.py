from dotenv import load_dotenv
import os

load_dotenv()


INFLUX_TOKEN = os.getenv('INFLUXDB_TOKEN')
INFLUX_ORG = os.getenv('INFLUXDB_ORG')
INFLUX_BUCKET = os.getenv('INFLUXDB_BUCKET')
INFLUX_URL = os.getenv('INFLUXDB_URL', 'http://localhost:8086')

MQTT_BROKER = os.getenv("MQTT_BROKER")
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883)) 
MQTT_TOPIC = "esp32/smartfarming/sensor"
