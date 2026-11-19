#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials: enter these locally; do not post them publicly.
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

// MQTT broker and separate publish/command topics
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "testtopic/temp/outTopic/miguel123";
const char* MQTT_IN_TOPIC = "testtopic/temp/inTopic/miguel123";

// NodeMCU pins
const int POT_PIN = A0;
const int BUTTON_PIN = D5;
const int LED_PIN = D6;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastPublish = 0;
const unsigned long publishInterval = 15000;
int lastButtonState = HIGH;
bool sendButtonRelease = false;
unsigned long buttonPressTime = 0;

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");

  IPAddress localIP(192, 168, 137, 190);
  IPAddress gateway(192, 168, 137, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns1(8, 8, 8, 8);
  IPAddress dns2(1, 1, 1, 1);

  WiFi.config(localIP, gateway, subnet, dns1, dns2);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (length == 1 && payload[0] == '1') {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("MQTT command: LED ON");
  } else if (length == 1 && payload[0] == '0') {
    digitalWrite(LED_PIN, LOW);
    Serial.println("MQTT command: LED OFF");
  }
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");

    String clientId = "NodeMCU-";
    clientId += String(ESP.getChipId(), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      mqttClient.subscribe(MQTT_IN_TOPIC);
      Serial.print("Subscribed to: ");
      Serial.println(MQTT_IN_TOPIC);
    } else {
      Serial.print("failed, state=");
      Serial.print(mqttClient.state());
      Serial.println("; retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  connectWiFi();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setSocketTimeout(30);
  mqttClient.setCallback(mqttCallback);

  Serial.println("Ready. Enter 1 or 0 in Serial Monitor to test the LED.");
  Serial.print("Publishing potentiometer data to: ");
  Serial.println(MQTT_TOPIC);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  mqttClient.loop();

  int potValue = analogRead(POT_PIN);
  int buttonState = digitalRead(BUTTON_PIN);

  // INPUT_PULLUP means LOW is pressed.
  if (lastButtonState == HIGH && buttonState == LOW) {
    if (mqttClient.publish(MQTT_TOPIC, "1")) {
      Serial.println("Button pressed: published 1");
      buttonPressTime = millis();
      sendButtonRelease = true;
    } else {
      Serial.println("Button message publish failed");
    }
  }

  if (sendButtonRelease && millis() - buttonPressTime >= 5000) {
    if (mqttClient.publish(MQTT_TOPIC, "0")) {
      Serial.println("Five seconds later: published 0");
    } else {
      Serial.println("Button release message publish failed");
    }
    sendButtonRelease = false;
  }

  lastButtonState = buttonState;

  Serial.print("Potentiometer: ");
  Serial.print(potValue);
  Serial.print(" | Button: ");
  Serial.println(buttonState == LOW ? 1 : 0);

  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == '1') {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
    } else if (command == '0') {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
    }
  }

  unsigned long now = millis();

  if (now - lastPublish >= publishInterval) {
    lastPublish = now;

    char payload[12];
    snprintf(payload, sizeof(payload), "%d", potValue);

    if (mqttClient.publish(MQTT_TOPIC, payload)) {
      Serial.print("Published potentiometer value: ");
      Serial.println(payload);
    } else {
      Serial.println("Publish failed");
    }
  }

  delay(250);
}
