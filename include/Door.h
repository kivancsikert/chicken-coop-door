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
    CLOSED = -2,
    CLOSING = -1,
    OPENING = 1,
    OPEN = 2
};

class Door
    : public TelemetryProvider,
      private ConfigAware,
      private Loopable<bool> {
public:
    Door(const Config& config, SwitchHandler& openSwitch, SwitchHandler& closedSwitch)
        : ConfigAware(config)
        , openSwitch(openSwitch)
        , closedSwitch(closedSwitch)
        , motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4) {
    }

    void begin(std::function<void(std::function<void(JsonObject&)>)> onEvent);

    /**
     * Loops the door, and returns whether the door is currently moving.
     */
    bool loop() override;

    void moveTo(long position) {
        motor.moveTo(position);
    }
    void override(GateState state) {
        this->manualOverride = true;
        onEvent([](JsonObject& json) { json["manualOverride"] = true; });
        startMoving(state);
    }
    void lightChanged(float light);
    void populateTelemetry(JsonObject& json) override {
        json["manualOverride"] = manualOverride;
        json["emergencyStop"] = emergencyStop;
        json["gate"] = static_cast<int>(state);
        json["motorPosition"] = motor.currentPosition();
    }

private:
    SwitchHandler& openSwitch;
    SwitchHandler& closedSwitch;
    AccelStepper motor;

    std::function<void(std::function<void(JsonObject&)>)> onEvent;

    /**
     * The state of the gate.
     */
    GateState state;

    /**
     * Ignore light levels, and keep open or closed until further notice.
     */
    bool manualOverride = false;

    /**
     * Is the door disabled because its movement timed out?
     */
    bool emergencyStop = false;

    void setState(GateState state) {
        this->state = state;
        onEvent([state](JsonObject& json) { json["state"] = static_cast<int>(state); });
    }

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

    void halt(const String& reason) {
        Serial.println("Emergency stopping: " + reason);
        emergencyStop = true;
        motor.stop();
        motor.disableOutputs();
        onEvent([reason](JsonObject& json) {
            json["emergencyStop"] = true;
            json["reason"] = reason;
        });
    }

    unsigned long movementStarted;
};
