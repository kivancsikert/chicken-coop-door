#include <Arduino.h>

#include <AccelStepper.h>
#include <BH1750.h>
#include <Wire.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include <ArduinoJson.h>

#define OPEN_PIN D5
#define CLOSED_PIN D6

const int stepsPerRevolution = 2048;

AccelStepper motor(AccelStepper::FULL4WIRE, D0, D2, D1, D3);

BH1750 lightMeter;

struct Config {
    /**
     * Light required to be above limit to open the door.
     */
    float openLightLimit = 120;

    /**
     * Light required to be below limit to open the door.
     */
    float closeLightLimit = 50;

    /**
     * Whether to invert the "gate open" switch or not.
     */
    bool invertOpenSwitch = false;

    /**
     * Whether to invert the "gate close" switch or not.
     */
    bool invertCloseSwitch = false;
} config;

enum class GateState {
    OPEN,
    CLOSED,
    OPENING,
    CLOSING
};

struct State {
    /**
     * The current level of light.
     */
    float currentLight = 0;

    /**
     * The state of the gate.
     */
    GateState gateState = GateState::OPEN;

    /**
     * Whether the "gate open" switch is engaged or not.
     */
    bool openSwitch = false;

    /**
     * Whether the "gate closed" switch is engaged or not.
     */
    bool closedSwitch = false;
} state;

AsyncWebServer server(80);

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(OPEN_PIN, INPUT);
    pinMode(CLOSED_PIN, INPUT);

    while (!Serial) {
        delay(100);
    }

    motor.setMaxSpeed(500);
    motor.setSpeed(500);
    motor.setAcceleration(500);

    Wire.begin(9, 10);
    if (lightMeter.begin()) {
        Serial.println("BH1750 initialised");
    } else {
        Serial.println("Error initialising BH1750");
    }

    WiFi.mode(WIFI_AP_STA);
    WiFi.hostname("chickens");
    delay(500);
    WiFi.beginSmartConfig();
    Serial.print("WiFi connecting via SmartConfig");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        Serial.println(WiFi.smartConfigDone());
    }
    Serial.print("WiFi connected, SSID: ");
    Serial.print(WiFi.SSID());
    Serial.print(", IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print(", hostname: ");
    Serial.println(WiFi.hostname());

    LittleFSConfig cfg;
    cfg.setAutoFormat(true);
    LittleFS.setConfig(cfg);
    LittleFS.begin();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncResponseStream* response = request->beginResponseStream("application/json");
        const int capacity = JSON_OBJECT_SIZE(10);
        StaticJsonDocument<capacity> results;
        results["version"] = "1.2.3";
        results["light"] = state.currentLight;
        results["openLightLimit"] = config.openLightLimit;
        results["closeLightLimit"] = config.closeLightLimit;
        results["motor_position"] = motor.currentPosition();
        results["openSwitch"] = state.openSwitch;
        results["closedSwitch"] = state.closedSwitch;
        results["gateState"] = static_cast<int>(state.gateState);
        serializeJson(results, *response);
        request->send(response);
    });

    server.begin();
}

unsigned long previousMillis = 0;
unsigned long interval = 1000;

unsigned int stepsAtOnce = 100;

void loop()
{
    state.openSwitch = digitalRead(OPEN_PIN) ^ config.invertOpenSwitch;
    state.closedSwitch = digitalRead(CLOSED_PIN) ^ config.invertCloseSwitch;

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        state.currentLight = lightMeter.readLightLevel();
        if (state.currentLight < config.closeLightLimit && state.gateState == GateState::OPEN) {
            Serial.println("Closing...");
            state.gateState = GateState::CLOSING;
        } else if (state.currentLight > config.openLightLimit && state.gateState == GateState::CLOSED) {
            Serial.println("Opening...");
            state.gateState = GateState::OPENING;
        }
        Serial.printf("Light: %f, state: %d\n", state.currentLight, state.gateState);
    }

    if (!motor.run()) {
        if (state.gateState == GateState::OPEN || state.gateState == GateState::CLOSED) {
            motor.disableOutputs();
            delay(250);
            return;
        }
    }

    if (state.gateState == GateState::CLOSING) {
        if (state.closedSwitch) {
            Serial.println("Closed");
            motor.stop();
            state.gateState = GateState::CLOSED;
        } else {
            motor.move(stepsAtOnce);
        }
    } else if (state.gateState == GateState::OPENING) {
        if (state.openSwitch) {
            motor.stop();
            state.gateState = GateState::OPEN;
        } else {
            motor.move(-stepsAtOnce);
        }
    }
}
