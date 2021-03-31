#pragma once

#include <AccelStepper.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>

#include "Config.h"
#include "LightHandler.h"
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
    Door(Config& config, MqttHandler& mqtt, LightHandler& light)
        : ConfigAware(config)
        , mqtt(mqtt)
        , light(light) {
    }

    void begin();

    /**
     * Loops the door, and returns whether the door is currently moving.
     */
    bool loop();

    void moveTo(long position);

private:
    MqttHandler& mqtt;
    LightHandler& light;

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

    /**
     * Updates the gate state and returns whether the motor is currently moving.
     */
    bool updateMotor();
    void publishTelemetry(unsigned long currentMillis);

    unsigned long previousLightUpdateMillis = 0;
    unsigned long previousStatePublishMillis = 0;
};
