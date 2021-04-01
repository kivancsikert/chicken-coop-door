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
        , light(light)
        , motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4) {
    }

    void begin();

    /**
     * Loops the door, and returns whether the door is currently moving.
     */
    bool loop();

    void moveTo(long position) {
        motor.moveTo(position);
    }
    void setState(GateState state) {
        this->state = state;
    }

private:
    MqttHandler& mqtt;
    LightHandler& light;
    AccelStepper motor;

    /**
     * The state of the gate.
     */
    GateState state = GateState::OPEN;

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

    unsigned long previousStatePublishMillis = 0;
};
