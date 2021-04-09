#pragma once

#include <AccelStepper.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>
#include <functional>

#include "Config.h"
#include "LightHandler.h"
#include "PinAllocation.h"
#include "SwitchHandler.h"
#include "Telemetry.h"

enum class GateState {
    OPEN,
    CLOSED,
    OPENING,
    CLOSING
};

class Door
    : public TelemetryProvider,
      private ConfigAware {
public:
    Door(const Config& config, LightHandler& light, SwitchHandler& openSwitch, SwitchHandler& closedSwitch)
        : ConfigAware(config)
        , light(light)
        , openSwitch(openSwitch)
        , closedSwitch(closedSwitch)
        , motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4) {
    }

    void begin(std::function<void(std::function<void(JsonObject&)>)> onEvent);

    /**
     * Loops the door, and returns whether the door is currently moving.
     */
    bool loop();

    void moveTo(long position) {
        motor.moveTo(position);
    }
    void setState(GateState state) {
        this->state = state;
        onEvent([state](JsonObject& json) { json["state"] = static_cast<int>(state); });
    }
    void populateTelemetry(JsonObject& json) override {
        json["emergencyStop"] = emergencyStop;
        json["gate"] = static_cast<int>(state);
        json["motorPosition"] = motor.currentPosition();
    }

private:
    LightHandler& light;
    SwitchHandler& openSwitch;
    SwitchHandler& closedSwitch;
    AccelStepper motor;

    std::function<void(std::function<void(JsonObject&)>)> onEvent;

    /**
     * The state of the gate.
     */
    GateState state = GateState::OPEN;

    /**
     * Is the door disabled because its movement timed out?
     */
    bool emergencyStop = false;

    /**
     * Starts to move the motor towards opening or closing.
     */
    void startMoving(GateState state) {
        movementStarted = millis();
        setState(state);
    }

    /**
     * Finishes moving in the given state.
     */
    void stopMoving(GateState state) {
        motor.stop();
        setState(state);
    }

    /**
     * Advances the motor in the given direction.
     */
    void advanceMotor(long steps);

    unsigned long movementStarted;
};
