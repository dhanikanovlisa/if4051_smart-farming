import paho.mqtt.client as mqtt
import threading
import os
import json
from influxdb_client import InfluxDBClient, Point, WriteOptions
from influxdb_client.client.write_api import SYNCHRONOUS
from config.config import MQTT_BROKER_IP, MQTT_BROKER_PORT, MQTT_TOPIC

# Global to store message
last_message = "No messages received yet."

# InfluxDB Config from environment
influx_url = os.getenv("INFLUX_URL")
token = os.getenv("INFLUX_TOKEN")
org = os.getenv("INFLUX_ORG")
bucket = os.getenv("INFLUX_BUCKET")

influx_client = InfluxDBClient(url=influx_url, token=token, org=org)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

# Callback for connect
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(MQTT_TOPIC)

# Callback for message
def on_message(client, userdata, msg):
    global last_message
    last_message = msg.payload.decode()
    print(f"MQTT Message Received: {last_message}")

    try:
        payload = json.loads(last_message)
        value = float(payload["temperature"])  # extract from JSON
        point = (
            Point("mqtt_measurement")  # consistent measurement name
            .field("field1", value)
        )
        write_api.write(bucket=bucket, org=org, record=point)
        print("✅ Written to InfluxDB")
    except (ValueError, KeyError, json.JSONDecodeError) as e:
        print(f"❌ Failed to write to InfluxDB: {e}")

# MQTT client setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

def start():
    def run():
        client.connect(MQTT_BROKER_IP, MQTT_BROKER_PORT, 60)
        client.loop_forever()
    thread = threading.Thread(target=run)
    thread.daemon = True
    thread.start()

def get_last_message():
    return last_message

def publish_message(message):
    client.publish(MQTT_TOPIC, message)
