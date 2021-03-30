#ifdef ESP32
#include "pins_arduino_ttgo_call.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <SPIFFS.h>
#elif defined(ESP8266)
#define SPIFFS LittleFS
#include <LittleFS.h>
#endif

#include <Arduino.h>

#include <ArduinoJson.h>

#include "door.h"
#include "gsm.h"
#include "mqtt-handler.h"
#include "ota.h"

Ota ota;

Config config;

Gsm gsm(config);

MqttHandler mqttHandler;

Door door(config, mqttHandler);

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

    config.begin();

    WiFi.mode(WIFI_AP_STA);
    delay(500);
    if (!config.wifiSsid.isEmpty()) {
        Serial.print("Using stored WIFI configuration to connect to ");
        Serial.print(config.wifiSsid);
        Serial.print("...");
        WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
    } else {
        Serial.print("WIFI is not configured, using SmartConfig...");
        bool smartConfigBeginSuccess = WiFi.beginSmartConfig();
        if (!smartConfigBeginSuccess) {
            Serial.print(" unsuccessful");
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

    gsm.begin();

    File iotConfigFile = SPIFFS.open("/iot-config.json", FILE_READ);
    DynamicJsonDocument iotConfigJson(iotConfigFile.size() * 2);
    DeserializationError error = deserializeJson(iotConfigJson, iotConfigFile);
    if (error) {
        Serial.printf("Failed to read IoT config file (%s)\n", error.c_str());
    }
    WiFiClientSecure* wifiClient = new WiFiClientSecure();
    wifiClient->setCACert(root_cert.c_str());
    mqttHandler.begin(
        wifiClient,
        iotConfigJson,
        [](const JsonDocument& json) {
            config.update(json);
            Serial.println("Received configuration");
            config.store();
            Serial.println("Stored configuration");
        },
        [](const JsonDocument& json) {
            door.executeCommand(json);
        });

    door.begin();
}

void loop() {
    ota.handle();
    mqttHandler.loop();
    door.loop();
}
