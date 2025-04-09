from flask import Flask, jsonify
from influxdb_client import InfluxDBClient, Point, WriteOptions
from influxdb_client.client.write_api import SYNCHRONOUS
import os
import time
import mqtt_client 

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

    points = []
    for value in range(5):
        point = (
            Point("measurement1")
            .tag("tagname1", "tagvalue1")
            .field("field1", value)
        )
        points.append(point)
        time.sleep(1)

    write_api.write(bucket="mybucket", org="myorg", record=points)

    return jsonify(message="Data written to InfluxDB successfully")

@app.route("/read_data")
def read_data():
    query_api = influx_client.query_api()

    query = """
    from(bucket: "mybucket")
    |> range(start: -10m)
    |> filter(fn: (r) => r._measurement == "mqtt_measurement")
    """
    
    tables = query_api.query(query, org="myorg")

    data = []
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

