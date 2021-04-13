#ifdef ESP32
#include "pins_arduino_ttgo_call.h"

#include <SPIFFS.h>
#elif defined(ESP8266)
#define SPIFFS LittleFS
#include <LittleFS.h>
#endif

#include <Arduino.h>

#include <ArduinoJson.h>

#include "DebugClient.h"
#include "Door.h"
#include "LightHandler.h"
#include "MqttHandler.h"
#include "OtaHandler.h"
#include "PinAllocation.h"
#include "SwitchHandler.h"
#include "Telemetry.h"
#include "WiFiHandler.h"
#include "google-iot-root-cert.h"
#include "version.h"

Config config;
OtaHandler ota;
WiFiHandler wifi(config);

LightHandler light(config);
SwitchHandler openSwitch("openSwitch", OPEN_PIN, []() { return config.invertOpenSwitch; });
SwitchHandler closedSwitch("closedSwitch", CLOSED_PIN, []() { return config.invertCloseSwitch; });
Door door(config, light, openSwitch, closedSwitch);

MqttHandler mqtt;
CompositeTelemetryProvider telemetryProvider({ &light, &openSwitch, &closedSwitch, &door });
TelemetryPublisher telemetryPublisher(config, mqtt, telemetryProvider);

String fatalError(String message) {
    Serial.println(message);
    delay(10000);
    ESP.restart();
    return "Should never get here";
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

    wifi.begin("chickens", googleIoTRootCert);
    ota.begin("chickens");

    File iotConfigFile = SPIFFS.open("/iot-config.json", FILE_READ);
    DynamicJsonDocument iotConfigJson(iotConfigFile.size() * 2);
    DeserializationError error = deserializeJson(iotConfigJson, iotConfigFile);
    if (error) {
        throw fatalError("Failed to read IoT config file: " + String(error.c_str()));
    }
    mqtt.begin(
        wifi.getClient(),
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

    light.begin(LIGHT_SDA, LIGHT_SCL);
    openSwitch.begin();
    closedSwitch.begin();
    door.begin([](std::function<void(JsonObject&)> populateEvent) {
        DynamicJsonDocument doc(2048);
        JsonObject root = doc.to<JsonObject>();
        root["version"] = VERSION;
        JsonObject event = root.createNestedObject("event");
        populateEvent(event);
        JsonObject telemetry = root.createNestedObject("telemetry");
        telemetryProvider.populateTelemetry(telemetry);
        mqtt.publishState(doc);
    });
    telemetryPublisher.begin();
}

void loop() {
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
