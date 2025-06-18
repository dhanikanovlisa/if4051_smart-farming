from dateutil import parser, tz
import paho.mqtt.client as mqttClient
import os
from influxdb_client import InfluxDBClient, Point, WritePrecision
import json  

mqtt_broker = "broker.emqx.io"
mqtt_topic = "esp32/smartfarming/sensor"

token = os.environ.get("INFLUXDB_TOKEN")
org = "f007004f738f5b7b"
url = os.environ.get("INFLUXDB_URL", "http://influxdb:8086")
bucket = os.environ.get("INFLUXDB_BUCKET", "mybucket")

print(token, org, url, bucket)
# InfluxDB Client setup
influx_client = InfluxDBClient(url=url, token=token, org=org)

# Create WriteApi instance
write_api = influx_client.write_api()

WIB_TZ = tz.gettz("Asia/Jakarta")
# === Callback when message is received ===
def on_message(client, userdata, msg):
    print(f"[MQTT] Message received on topic {msg.topic}")    
    try:
        # Decode the MQTT payload and parse it as JSON
        payload = msg.payload.decode('utf-8')
        data = json.loads(payload)  # Convert string to dictionary
        
        # Extract the data fields
        print(f"[MQTT] Data: {data}")
        sensor_id = data["sensor_id"]
        temperature = data["temperature"]
        humidity = data["humidity"]
        moisture_percent = float(data["moisture_percent"])
        lux = data["lux"]
        naive_timestamp = parser.isoparse(data["timestamp"])
        timestamp_with_tz = naive_timestamp.replace(tzinfo=WIB_TZ)

        # InfluxDB expects UTC. Convert the timezone-aware timestamp to UTC
        timestamp_utc = timestamp_with_tz.astimezone(tz.gettz("UTC"))


        # Prepare data point for InfluxDB
        point = (
            Point("sensor_data")
            .tag("sensor_id", sensor_id)
            .field("temperature", temperature)
            .field("humidity", humidity)
            .field("moisture_percent", moisture_percent)
            .field("lux", lux)
            .time(timestamp_utc, WritePrecision.NS)  # Use nanoseconds for timestamp
        )

        # Write the point to InfluxDB
        write_api.write(bucket=bucket, org=org, record=point)
        print("✅ Written to InfluxDB")
                
    except Exception as e:
        print("[ERROR] Failed to process message:", e)

# === Main Subscriber Setup ===
client = mqttClient.Client(mqttClient.CallbackAPIVersion.VERSION2, client_id="mqtt-subscriber")
client.connect(mqtt_broker, 1883, 60)
client.subscribe(mqtt_topic)
client.on_message = on_message

print("[MQTT] Subscribed. Waiting for messages...")
client.loop_forever()
