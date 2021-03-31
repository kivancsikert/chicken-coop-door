#pragma once

#include <AccelStepper.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>

#include "Config.h"
#include "MqttHandler.h"
#include "PinAllocation.h"

enum class GateState {
    OPEN,
    CLOSED,
    OPENING,
    CLOSING
};

class Door
    : ConfigAware {
public:
    Door(Config& config, MqttHandler& mqtt)
        : ConfigAware(config)
        , mqtt(mqtt) {
    }

    void begin();

    /**
     * Loops the door, and returns whether the door is currently moving.
     */
    bool loop();

    void executeCommand(const JsonDocument& json);

private:
    MqttHandler& mqtt;

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

    /**
     * Updates the gate state and returns whether the motor is currently moving.
     */
    bool updateMotor();
    void publishTelemetry(unsigned long currentMillis);

    unsigned long previousLightUpdateMillis = 0;
    unsigned long previousStatePublishMillis = 0;
};
