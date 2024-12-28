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

#include <ArduinoHA.h>

#define BROKER_ADDR IPAddress(192, 168, 1, 102)
#define BROKER_USERNAME "mosquito"
#define BROKER_PASSWORD "ys2yGT32kdgwy8"

byte mac[] = { 0xC0, 0x4B, 0x30, 0x12, 0x06, 0x6C };


#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)

//pins
const int totalSensors = 4;
const int sensorPinsDigital[totalSensors] = { 2, 3, 4, 5 };
const int sensorPinsAnalog[totalSensors] = { A2, A3, A4, A5 };
int sensorStateDigital[totalSensors];
uint16_t sensorStateAnalog[totalSensors];

WiFiClient wifiClient;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(wifiClient, device);

HASensor sensors[] = {
  HASensor("moist1"),
  HASensor("moist2"),
  HASensor("moist3"),
  HASensor("moist4")
};

const long updateInterval = 1000;
unsigned long lastUpdateAt = 0;
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
    Serial.print("retrying...");
    delay(5000);
  }

  Serial.println("WiFi connected");

  mqtt.setDataPrefix("fp");

  // set device details
  device.setName("Flower Power");
  device.setSoftwareVersion("0.0.1");
  device.enableSharedAvailability();
  device.enableLastWill();

  // configure sensors
  for (HASensor sensor : sensors) {
    sensor.setIcon("mdi:water");
    sensor.setDeviceClass("sensor");
  }
  // moistureSensor1.setIcon("mdi:water");
  // moistureSensor1.setName("Moisture Sensor 1");
  // moistureSensor1.setDeviceClass("sensor");

  // moistureSensor2.setIcon("mdi:water");
  // moistureSensor2.setName("Moisture Sensor 2");
  // moistureSensor2.setDeviceClass("sensor");

  // moistureSensor3.setIcon("mdi:water");
  // moistureSensor3.setName("Moisture Sensor 3");
  // moistureSensor3.setDeviceClass("sensor");

  // moistureSensor4.setIcon("mdi:water");
  // moistureSensor4.setName("Moisture Sensor 4");
  // moistureSensor4.setDeviceClass("sensor");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("mosquito", "ys2yGT32kdgwy8");
  mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
}

void loop() {
  mqtt.loop();

  if ((millis() - lastUpdateAt) > updateInterval) {
    for (int i = 0; i < totalSensors; i++) {

      // for digital: 1 = out of the dirt, 0 = in dirt
      // analog ranges: wet = 0-300, dry = 301 - 400, arid: 401+
      sensorStateDigital[i] = digitalRead(sensorPinsDigital[i]);
      sensorStateAnalog[i] = analogRead(sensorPinsAnalog[i]);
    }

    for (int j = 0; j < totalSensors; j++) {
      if (sensorStateDigital[j] == 0) {  // it is in dirt, is it wet?
        if (sensorStateAnalog[j] < 300) {
          // all good
          sensors[j].setValue("moist");
        } else if (sensorStateAnalog[j] > 300 && sensorStateAnalog[j] < 400) {
          // check soil
          sensors[j].setValue("ok");
        } else {
          // water now!
          sensors[j].setValue("dry");
        }
      } else {  // not in dirt, send alert
        sensors[j].setValue("alert");
      }
    }

    // moistureSensor1.setValue("good");
    lastUpdateAt = millis();
  }
}