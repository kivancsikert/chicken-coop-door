#ifdef ESP32
#include "pins_arduino_ttgo_call.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <SPIFFS.h>
#elif defined(ESP8266)
#define SPIFFS LittleFS
#include <LittleFS.h>
#endif

#include "ota.h"
#include <Arduino.h>

#include <ArduinoJson.h>

#include "door.h"
#include "mqtt-handler.h"

Ota ota;

Door door;

void LOG(const char* message) {
    Serial.println(message);
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);

    while (!Serial) {
        delay(100);
    }

    Serial.println("Starting up file system...");
#ifdef ESP32
    if (!SPIFFS.begin()) {
        Serial.println("Failed.");
    }

    Serial.println("Contents:");
    File root = SPIFFS.open("/");
    while (true) {
        File file = root.openNextFile();
        if (!file) {
            break;
        }
        Serial.print(" - ");
        Serial.println(file.name());
    }
#elif defined(ESP8266)
    LittleFSConfig cfg;
    cfg.setAutoFormat(true);
    LittleFS.setConfig(cfg);
    LittleFS.begin();
#endif

    WiFi.mode(WIFI_AP_STA);
    delay(500);
    if (SPIFFS.exists("/wifi.txt")) {
        File wifiConfig = SPIFFS.open("/wifi.txt", FILE_READ);
        String ssid = wifiConfig.readStringUntil('\n');
        String password = wifiConfig.readStringUntil('\n');
        wifiConfig.close();
        Serial.print("Using stored WIFI configuration to connect to ");
        Serial.print(ssid);
        Serial.print("...");
        WiFi.begin(ssid.c_str(), password.c_str());
    } else {
        Serial.print("Couldn't find WIFI config, using SmartConfig...");
        bool smartConfigBeginSuccess = WiFi.beginSmartConfig();
        if (!smartConfigBeginSuccess) {
            Serial.println(" unsuccessful");
        }
    }
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        // Serial.print(WiFi.smartConfigDone());
        // Serial.print(" ");
        // Serial.println(WiFi.status());
    }
    WiFi.softAPsetHostname("chickens");
    Serial.print(" connected, SSID: ");
    Serial.print(WiFi.SSID());
    Serial.print(", IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print(", hostname: ");
    Serial.println(WiFi.softAPgetHostname());

    ota.begin("chickens");

    File iotConfigFile = SPIFFS.open("/iot-config.json", FILE_READ);
    DynamicJsonDocument iotConfigJson(iotConfigFile.size() * 2);
    DeserializationError error = deserializeJson(iotConfigJson, iotConfigFile);
    if (error) {
        Serial.printf("Failed to read IoT config file (%s)\n", error.c_str());
    }
    WiFiClientSecure* wifiClient = new WiFiClientSecure();
    wifiClient->setCACert(root_cert.c_str());
    mqttHandler.begin(wifiClient, iotConfigJson);

    door.begin();
}

void loop() {
    ota.handle();
    mqttHandler.loop();
    door.loop();
}
