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
#include "SwitchHandler.h"
#include "Telemetry.h"
#include "WiFiHandler.h"
#include "google-iot-root-cert.h"
#include "version.h"

OtaHandler ota;
Config config;
WiFiHandler wifi(config);
GprsHandler gprs(config);
MqttHandler mqtt;
LightHandler light(config);
SwitchHandler openSwitch(OPEN_PIN, []() { return config.invertOpenSwitch; });
SwitchHandler closedSwitch(CLOSED_PIN, []() { return config.invertCloseSwitch; });
Door door(config, mqtt, light, openSwitch, closedSwitch);
TelemetryPublisher telemetryPublisher(config, mqtt, { &light, &door });

String fatalError(String message) {
    Serial.println(message);
    delay(10000);
    ESP.restart();
    return "Should never get here";
}

Client& chooseMqttConnection() {
    if (gprs.begin(googleIoTRootCert)) {
        Serial.println("GPRS available, using it for MQTT");
        return gprs.getClient();
    } else if (config.wifiEnabled) {
        Serial.println("GPRS not available, falling back to WIFI for MQTT");
        return wifi.getClient();
    } else {
        throw fatalError("Neither WIFI nor GPRS available, restarting");
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
        throw fatalError("Could not initialize file system");
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

    if (config.wifiEnabled) {
        wifi.begin("chickens");
        ota.begin("chickens");
    }

    File iotConfigFile = SPIFFS.open("/iot-config.json", FILE_READ);
    DynamicJsonDocument iotConfigJson(iotConfigFile.size() * 2);
    DeserializationError error = deserializeJson(iotConfigJson, iotConfigFile);
    if (error) {
        throw fatalError("Failed to read IoT config file: " + String(error.c_str()));
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
    openSwitch.begin();
    closedSwitch.begin();
    door.begin();
    telemetryPublisher.begin();
}

void loop() {
    // It's okay to loop OTA unconditionally, it will ignore the call if not initialized
    ota.loop();
    light.loop();
    openSwitch.loop();
    closedSwitch.loop();
    bool moving = door.loop();
    // Preserve power by making sure we are not transmitting while the door is moving
    if (!moving) {
        telemetryPublisher.loop();
        mqtt.loop();
    }
}
