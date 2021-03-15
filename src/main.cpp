#ifdef ESP32
#include "pins_arduino_ttgo_call.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#define SPIFFS LittleFS
#include <LittleFS.h>
#endif
#include <ESPAsyncWebServer.h>

#include <Arduino.h>
#include <ArduinoOTA.h>

#include <AccelStepper.h>
#include <BH1750.h>
#include <Wire.h>

#include <ArduinoJson.h>

#ifdef ESP32

#define MOTOR_PIN1 GPIO_NUM_32
#define MOTOR_PIN2 GPIO_NUM_33
#define MOTOR_PIN3 GPIO_NUM_25
#define MOTOR_PIN4 GPIO_NUM_26

#define LIGHT_SDA  GPIO_NUM_13
#define LIGHT_SCL  GPIO_NUM_15

#define OPEN_PIN   GPIO_NUM_14
#define CLOSED_PIN GPIO_NUM_12

#elif defined(ESP8266)

#define OPEN_PIN D5
#define CLOSED_PIN D6

#define MOTOR_PIN1 D0
#define MOTOR_PIN2 D1
#define MOTOR_PIN3 D2
#define MOTOR_PIN4 D3

#define LIGHT_SDA D7
#define LIGHT_SDC D4

#endif

const int stepsPerRevolution = 2048;

AccelStepper motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

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
    bool invertOpenSwitch = true;

    /**
     * Whether to invert the "gate close" switch or not.
     */
    bool invertCloseSwitch = true;
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
AsyncWebSocket webSocket("/log");

void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
    void* arg, uint8_t* data, size_t length)
{
    switch (type) {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA: {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && (info->index == 0) && (info->len == length)) {
            if (info->opcode == WS_TEXT) {
                DynamicJsonDocument request(length + 1);
                deserializeJson(request, (char*)data);
                if (request.containsKey("command")) {
                    String command = request["command"];
                    if (command == "update") {
                        if (request.containsKey("gateState")) {
                            state.gateState = static_cast<GateState>((int) request["gateState"]);
                        }
                        if (request.containsKey("motorPosition")) {
                            motor.moveTo(request["motorPosition"]);
                        }
                    } else if (command == "status") {
                        const int capacity = JSON_OBJECT_SIZE(10);
                        StaticJsonDocument<capacity> message;
                        message["type"] = "status";
                        message["version"] = "1.2.7";
                        message["light"] = state.currentLight;
                        message["openLightLimit"] = config.openLightLimit;
                        message["closeLightLimit"] = config.closeLightLimit;
                        message["motorPosition"] = motor.currentPosition();
                        message["openSwitch"] = state.openSwitch;
                        message["closedSwitch"] = state.closedSwitch;
                        message["gateState"] = static_cast<int>(state.gateState);
                        char buffer[1024];
                        serializeJson(message, buffer);
                        client->text(buffer);
                    }
                }
            } else {
                Serial.println("Received a ws message, but it isn't text");
            }
        } else {
            Serial.println("Received a ws message, but it didn't fit into one frame");
        }
    } break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void LOG(const char* message)
{
    webSocket.printfAll("{\"type\": \"log\", \"message\": \"%s\"}", message);
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(OPEN_PIN, INPUT_PULLUP);
    pinMode(CLOSED_PIN, INPUT_PULLUP);
    pinMode(LIGHT_SDA, INPUT);
    pinMode(LIGHT_SCL, INPUT);

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

    motor.setMaxSpeed(500);
    motor.setSpeed(500);
    motor.setAcceleration(500);

    Wire.begin(LIGHT_SDA, LIGHT_SCL);
    Serial.println("Connecting to light sensor...");
    if (lightMeter.begin()) {
        Serial.println("BH1750 initialised");
    } else {
        Serial.println("Error initialising BH1750");
    }

    WiFi.mode(WIFI_AP_STA);
    delay(500);
    WiFi.beginSmartConfig();
    Serial.printf("WiFi connecting via SmartConfig...");
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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("Request for main page...");
        request->send(SPIFFS, "/index.html", "text/html");
    });

    webSocket.onEvent(onWebSocketEvent);
    server.addHandler(&webSocket);

    server.begin();

    ArduinoOTA.setHostname("chickens");
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Web socket error[%u]\n", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        } else {
            Serial.println("Other error");
        }
    });
    ArduinoOTA.begin();
}

unsigned long previousMillis = 0;
unsigned long interval = 1000;

unsigned int stepsAtOnce = 100;

void loop()
{
    ArduinoOTA.handle();
    webSocket.cleanupClients();

    state.openSwitch = digitalRead(OPEN_PIN) ^ config.invertOpenSwitch;
    state.closedSwitch = digitalRead(CLOSED_PIN) ^ config.invertCloseSwitch;

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        state.currentLight = lightMeter.readLightLevel();
        if (state.currentLight < config.closeLightLimit && state.gateState == GateState::OPEN) {
            LOG("Closing...");
            state.gateState = GateState::CLOSING;
        } else if (state.currentLight > config.openLightLimit && state.gateState == GateState::CLOSED) {
            LOG("Opening...");
            state.gateState = GateState::OPENING;
        }
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
            LOG("Closed");
            motor.stop();
            state.gateState = GateState::CLOSED;
        } else {
            motor.move(stepsAtOnce);
        }
    } else if (state.gateState == GateState::OPENING) {
        if (state.openSwitch) {
            LOG("Open");
            motor.stop();
            state.gateState = GateState::OPEN;
        } else {
            motor.move(-stepsAtOnce);
        }
    }
}
