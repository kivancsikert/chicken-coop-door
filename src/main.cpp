#ifdef ESP32
#include "pins_arduino_ttgo_call.h"
#endif

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Door.h"
#include "FileSystemHandler.h"
#include "HttpUpdateHandler.h"
#include "LightHandler.h"
#include "MqttHandler.h"
#include "NtpHandler.h"
#include "OtaHandler.h"
#include "SwitchHandler.h"
#include "Telemetry.h"
#include "WiFiHandler.h"
#include "google-iot-root-cert.h"
#include "version.h"

FileSystemHandler fileSystem;
Config config(fileSystem);
OtaHandler ota;
WiFiHandler wifi(config, "chickens", googleIoTRootCert);

LightHandler light(config);
SwitchHandler openSwitch("openSwitch", OPEN_PIN, []() { return config.invertOpenSwitch; });
SwitchHandler closedSwitch("closedSwitch", CLOSED_PIN, []() { return config.invertCloseSwitch; });
Door door(config, openSwitch, closedSwitch);

NtpHandler ntp(wifi);
MqttHandler mqtt(wifi, ntp);
HttpUpdateHandler httpUpdateHandler(wifi);
CompositeTelemetryProvider telemetryProvider({ &light, &openSwitch, &closedSwitch, &door });
TelemetryPublisher telemetryPublisher(config, mqtt, telemetryProvider);

void fatalError(String message) {
    Serial.println(message);
    delay(10000);
    ESP.restart();
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    while (!Serial) {
        delay(100);
    }

    Serial.println("Starting up file system...");
    if (!fileSystem.begin()) {
        fatalError("Could not initialize file system");
        return;
    }

    bool reset = digitalRead(RESET_BUTTON_PIN) == LOW;
    config.begin(reset);

    wifi.begin();
    ota.begin("chickens");

    File iotConfigFile = fileSystem.getFS().open("/iot-config.json", FILE_READ);
    DynamicJsonDocument iotConfigJson(iotConfigFile.size() * 2);
    DeserializationError error = deserializeJson(iotConfigJson, iotConfigFile);
    if (error) {
        fatalError("Failed to read IoT config file: " + String(error.c_str()));
        return;
    }
    mqtt.begin(
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
            if (json.containsKey("override")) {
                int targetStateValue = json["override"];
                GateState targetState = static_cast<GateState>(targetStateValue);
                Serial.println("Setting door state to " + String(targetStateValue));
                door.override(targetState);
            }
            if (json.containsKey("update")) {
                String url = json["update"];
                httpUpdateHandler.update(url, VERSION);
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
        mqtt.publishStatus(doc);
    });
    light.setOnUpdate([](float light) {
        door.lightChanged(light);
    });
    telemetryPublisher.begin();
}

void loop() {
    ota.loop();
    light.loop();
    openSwitch.loop();
    closedSwitch.loop();
    bool moving = door.loop();
    telemetryPublisher.loop();
    ntp.loop();
    mqtt.loop();
    if (!moving) {
        delay(1000);
    }
}
