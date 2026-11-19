import paho.mqtt.client as mqtt
import requests

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "testtopic/temp/outTopic/miguel123"

INSERT_URL = (
    "https://olivedrab-cobra-696323.hostingersite.com"
    "/database/insert_pot.php"
)
NODE_NAME = "node_1"

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Connected to MQTT broker")
        client.subscribe(TOPIC, qos=0)
        print(f"Subscribed to {TOPIC}")
    else:
        print(f"MQTT connection failed: {reason_code}")

def on_message(client, userdata, message):
    try:
        value_text = message.payload.decode("utf-8").strip()
        value = int(value_text)

        if not 0 <= value <= 1023:
            raise ValueError("potentiometer reading must be from 0 to 1023")

        print(f"Received potentiometer value: {value}")

        response = requests.post(
            INSERT_URL,
            data={"node_name": NODE_NAME, "pot_value": value},
            timeout=10,
        )
        print(f"Database response ({response.status_code}): {response.text}")

    except (UnicodeDecodeError, ValueError) as error:
        print(f"Ignoring invalid MQTT payload: {error}")
    except requests.RequestException as error:
        print(f"Could not send reading to Hostinger: {error}")

client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    client_id="miguel-python-relay",
)
client.on_connect = on_connect
client.on_message = on_message

print("Starting MQTT-to-Hostinger relay...")
client.connect(BROKER, PORT, keepalive=60)
client.loop_forever()
