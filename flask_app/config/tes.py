import paho.mqtt.client as mqtt
import time
import socket
from config.config import MQTT_BROKER_IP, MQTT_BROKER_PORT

class MQTTClient:
    def __init__(self):
        self.client = mqtt.Client(client_id="flask_client", clean_session=False)
        self.client.on_connect = self.on_connect
        self.client.on_disconnect = self.on_disconnect
        self.connected = False
        
        # Configure automatic reconnection
        self.client.reconnect_delay_set(min_delay=1, max_delay=120)
        
    def on_connect(self, client, userdata, flags, rc):
        self.connected = True
        print(f"✅ Connected to MQTT (rc: {rc})")
        client.subscribe("esp32/#")
        client.subscribe("rpi/broadcast")

    def on_disconnect(self, client, userdata, rc):
        self.connected = False
        print(f"❌ Disconnected (rc: {rc})")
        if rc != 0:
            print("Unexpected disconnection. Will auto-reconnect...")

    def connect(self):
        while True:
            try:
                self.client.connect(MQTT_BROKER_IP, MQTT_BROKER_PORT, keepalive=60)
                return True
            except (socket.error, ConnectionRefusedError) as e:
                print(f"Connection failed: {e}. Retrying in 5s...")
                time.sleep(5)
            except Exception as e:
                print(f"Unexpected error: {e}")
                return False

def init_mqtt():
    mqtt_client = MQTTClient()
    if mqtt_client.connect():
        mqtt_client.client.loop_start()
        return mqtt_client.client
    return None