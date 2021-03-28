#pragma once

#include <AccelStepper.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>

#include "config.h"
#include "mqtt-handler.h"

#ifdef ESP32

#define MOTOR_PIN1 GPIO_NUM_32
#define MOTOR_PIN2 GPIO_NUM_33
#define MOTOR_PIN3 GPIO_NUM_25
#define MOTOR_PIN4 GPIO_NUM_26

#define LIGHT_SDA GPIO_NUM_13
#define LIGHT_SCL GPIO_NUM_15

#define OPEN_PIN GPIO_NUM_14
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

enum class GateState {
    OPEN,
    CLOSED,
    OPENING,
    CLOSING
};

class Door {
public:
    Door(Config& config, MqttHandler& mqttHandler);
    void begin();
    void loop();

    void executeCommand(const JsonDocument& json);

private:
    Config& config;
    MqttHandler& mqttHandler;

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

    void updateLight(unsigned long currentMillis);
    void updateMotor();
    void publishTelemetry(unsigned long currentMillis);

    unsigned long previousLightUpdateMillis = 0;
    unsigned long previousStatePublishMillis = 0;
};
