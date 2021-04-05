#pragma once

#include <AccelStepper.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>

#include "Config.h"
#include "LightHandler.h"
#include "MqttHandler.h"
#include "PinAllocation.h"
#include "SwitchHandler.h"

enum class GateState {
    OPEN,
    CLOSED,
    OPENING,
    CLOSING
};

class Door
    : ConfigAware {
public:
    Door(const Config& config, MqttHandler& mqtt, LightHandler& light, SwitchHandler& openSwitch, SwitchHandler& closedSwitch)
        : ConfigAware(config)
        , mqtt(mqtt)
        , light(light)
        , openSwitch(openSwitch)
        , closedSwitch(closedSwitch)
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
    SwitchHandler& openSwitch;
    SwitchHandler& closedSwitch;
    AccelStepper motor;

    /**
     * The state of the gate.
     */
    GateState state = GateState::OPEN;

    /**
     * Is the door disabled because its movement timed out?
     */
    bool emergencyStop = false;

    /**
     * Updates the gate state and returns whether the motor is currently moving.
     */
    bool updateMotor(unsigned long currentMillis);

    /**
     * Starts to move the motor towards opening or closing.
     */
    void startMoving(GateState state);

    /**
     * Advances the motor in the given direction.
     */
    void advanceMotor(unsigned long currentMillis, long steps);

    void publishTelemetry(unsigned long currentMillis);

    unsigned long previousStatePublishMillis = 0;

    unsigned long movementStarted;
};
