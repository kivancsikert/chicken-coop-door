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
#include "Door.h"
#include "GprsHandler.h"
#include "LightHandler.h"
#include "MqttHandler.h"
#include "OtaHandler.h"
#include "PinAllocation.h"
#include "WiFiHandler.h"
#include "google-iot-root-cert.h"
#include "version.h"

OtaHandler ota;
Config config;
WiFiHandler wifi(config);
GprsHandler gprs(config);
MqttHandler mqtt;
LightHandler light(config);
Door door(config, mqtt, light);

Client& chooseMqttConnection() {
    if (gprs.begin(googleIoTRootCert)) {
        Serial.println("GPRS available, using it for MQTT");
        return gprs.getClient();
    } else {
        Serial.println("GPRS not available, falling back to WIFI for MQTT");
        return wifi.getClient();
    }
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

    wifi.begin("chickens");

    ota.begin("chickens");

    File iotConfigFile = SPIFFS.open("/iot-config.json", FILE_READ);
    DynamicJsonDocument iotConfigJson(iotConfigFile.size() * 2);
    DeserializationError error = deserializeJson(iotConfigJson, iotConfigFile);
    if (error) {
        Serial.printf("Failed to read IoT config file (%s)\n", error.c_str());
    }
    Client& client = chooseMqttConnection();
    mqtt.begin(
        client,
        iotConfigJson,
        [](const JsonDocument& json) {
            config.update(json);
            config.store();
            Serial.println("Updated local configuration");
        },
        [](const JsonDocument& json) {
            if (json.containsKey("restart")) {
                Serial.println("Restart command received, restarting");
                ESP.restart();
            }
            if (json.containsKey("moveTo")) {
                long targetPosition = json["moveTo"];
                Serial.println("Moving door to " + String(targetPosition));
                door.moveTo(targetPosition);
            }
            if (json.containsKey("setState")) {
                int targetStateValue = json["setState"];
                GateState targetState = static_cast<GateState>(targetStateValue);
                Serial.println("Setting door state to " + String(targetStateValue));
                door.setState(targetState);
            }
        });

    DynamicJsonDocument stateJson(2048);
    stateJson["version"] = VERSION;
    mqtt.publishState(stateJson);

    light.begin(LIGHT_SDA, LIGHT_SCL);
    door.begin();
}

void loop() {
    ota.loop();
    light.loop();
    bool moving = door.loop();
    // Preserve power by making sure we are not transmitting while the door is moving
    if (!moving) {
        mqtt.loop();
    }
}
