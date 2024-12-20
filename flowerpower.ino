#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
#include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_GIGA) || defined(ARDUINO_OPTA)
#include <WiFi.h>
#elif defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOR4_WIFI)
#include <WiFiS3.h>
#endif

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)

//pins
const int totalSensors = 4;
int sensorPinsDigital[totalSensors] = { 2, 3, 4, 5 };
int sensorPinsAnalog[totalSensors] = { A0, A1, A2, A3 };

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "homeassistant.local";
int port = 1883;
const char topicDigital[] = "flowerpower/digital";
const char topicAnalog[] = "flowerpower/analog";

const long interval = 1000;
unsigned long previousMillis = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  for (int digitalPin : sensorPinsDigital) {
    pinMode(digitalPin, INPUT);
  }

  // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  IPAddress ip = WiFi.localIP();
  Serial.print("You're connected to the network. IP:");
  Serial.println(ip);

  // Each client must have a unique client ID
  mqttClient.setId("flowerpower");

  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword("mosquito", "ys2yGT32kdgwy8");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  int sensorStateDigital[totalSensors];
  for (int d = 0; d < totalSensors; d++) {
    sensorStateDigital[d] = digitalRead(sensorPinsDigital[d]);
  }

  int sensorStateAnalog[totalSensors];
  for (int a = 0; a < totalSensors; a++) {
    sensorStateAnalog[a] = analogRead(sensorPinsAnalog[a]);
  }

  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    // send message, the Print interface can be used to set the message contents
    Serial.println("Sending digital topic...");
    mqttClient.beginMessage(topicDigital);
    for (int sensor : sensorStateDigital) {
      mqttClient.print(sensor);
      delay(1);
    }
    mqttClient.endMessage();

    Serial.println("Sending analog topic...");
    mqttClient.beginMessage(topicAnalog);
    for (int sensor : sensorStateAnalog) {
      mqttClient.print(sensor);
      delay(1);
    }
    mqttClient.endMessage();

    Serial.println();
  }
}