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

#include "DebugClient.h"
#include "door.h"
#include "google-iot-root-cert.h"
#include "gsm.h"
#include "mqtt-handler.h"
#include "ota.h"
#include "version.h"

Ota ota;

Config config;

Gsm gsm(config);

MqttHandler mqttHandler;

Door door(config, mqttHandler);

DebugClient debugClient;

WiFiClient wifiClient;

Client& chooseMqttConnection() {
    if (gsm.begin(googleIoTRootCert)) {
        Serial.println("GPRS available, using it for MQTT");
        return gsm.getClient();
    } else {
        Serial.println("GPRS not available, falling back to WIFI for MQTT");
        return wifiClient;
    }
}

void startWifi() {
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
}

bool awaitConnect() {
    unsigned long startTime = millis();
    while (true) {
        wl_status_t status = WiFi.status();
        switch (status) {
            case WL_CONNECTED:
                return true;
            case WL_CONNECT_FAILED:
                Serial.println("WIFI connection failed");
                return false;
            default:
                break;
        }
        // TODO Make WIFI connection timeout configurable
        if (millis() - startTime > 10 * 1000) {
            Serial.println("WIFI connection timed out");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
}

void connectWifi() {
    bool wifiModeSuccessful = WiFi.mode(WIFI_AP_STA);
    if (!wifiModeSuccessful) {
        Serial.println("WIFI mode unsuccessful");
    }
    delay(500);
    int retries = 3;
    while (true) {
        startWifi();
        if (awaitConnect()) {
            break;
        }
        WiFi.stopSmartConfig();
        if (--retries > 0) {
            continue;
        }
        Serial.println("Failed to connect WIFI, restarting");
        ESP.restart();
    }

    WiFi.softAPsetHostname("chickens");
    Serial.print(" connected, IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print(", hostname: ");
    Serial.println(WiFi.softAPgetHostname());

    ota.begin("chickens");
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

    connectWifi();

    File iotConfigFile = SPIFFS.open("/iot-config.json", FILE_READ);
    DynamicJsonDocument iotConfigJson(iotConfigFile.size() * 2);
    DeserializationError error = deserializeJson(iotConfigJson, iotConfigFile);
    if (error) {
        Serial.printf("Failed to read IoT config file (%s)\n", error.c_str());
    }
    Client& client = chooseMqttConnection();
    mqttHandler.begin(
        client,
        iotConfigJson,
        [](const JsonDocument& json) {
            config.update(json);
            config.store();
            Serial.println("Updated local configuration");
        },
        [](const JsonDocument& json) {
            door.executeCommand(json);
        });

    DynamicJsonDocument stateJson(2048);
    stateJson["version"] = VERSION;
    mqttHandler.publishState(stateJson);

    door.begin();
}

void loop() {
    ota.handle();
    mqttHandler.loop();
    door.loop();
}
