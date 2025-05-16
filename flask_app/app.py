from flask import Flask, jsonify
from influxdb_client import InfluxDBClient, Point, WriteOptions
from influxdb_client.client.write_api import SYNCHRONOUS
import os
import time
from datetime import datetime, timezone
import mqtt_client 
import random
# Flask App
app = Flask(__name__)

mqtt_client.start()

# InfluxDB Configuration
influx_url = os.getenv("INFLUX_URL")
token = os.getenv("INFLUX_TOKEN")
org = os.getenv("INFLUX_ORG")
bucket = os.getenv("INFLUX_BUCKET")

influx_client = InfluxDBClient(url=influx_url, token=token, org=org)
write_api = influx_client.write_api()


@app.route("/")
def hello():
    return jsonify(message="Flask connected to InfluxDB and MQTT")

@app.route("/test_topic")
def write_data_mqtt():
    mqtt_client.publish("esp32/sensor2", "Hello from Flask!")
    return jsonify(message="Data sent to MQTT successfully")

@app.route("/subscribe")

@app.route("/write_data")
def write_data():
    write_api = influx_client.write_api(write_options=SYNCHRONOUS)

    for i in range(5):
        payload = {
            "sensor_id": "node_1",
            "temperature": round(25 + random.uniform(-2, 2), 2),
            "humidity": round(60 + random.uniform(-5, 5), 2),
            "moisture_percent": round(40 + random.uniform(-5, 5), 2),
            "lux": round(700 + random.uniform(-100, 100), 2),
            "timestamp": datetime.utcnow().replace(tzinfo=timezone.utc)
        }

        point = (
            Point("sensor_data")
            .tag("sensor_id", payload["sensor_id"])
            .field("temperature", payload["temperature"])
            .field("humidity", payload["humidity"])
            .field("moisture_percent", payload["moisture_percent"])
            .field("lux", payload["lux"])
            .time(payload["timestamp"])
        )

    write_api.write(bucket=bucket, org="myorg", record=point)
    print(f"✅ Data written at {payload['timestamp']}")
    time.sleep(1)  # Simulate delay between data points

    return jsonify(message="Payload-based data written to InfluxDB successfully")

@app.route("/read_data")
def read_data():
    query_api = influx_client.query_api()

    query = """
    from(bucket: "mybucket")
    |> range(start: -1h)
    |> filter(fn: (r) => r._measurement == "sensor_data")
    |> filter(fn: (r) => r["sensor_id"] == "node_1")
    """

    
    tables = query_api.query(query, org="myorg")
    data = []
    print(data)
    for table in tables:
        for record in table.records:
            data.append({
                "time": str(record.get_time()),
                "field": record.get_field(),
                "value": record.get_value(),
                "measurement": record.get_measurement(),
                "tags": record.values
            })
    
    return jsonify(message="Data read from InfluxDB successfully", data=data)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)

