#pragma once

#include <ArduinoJson.h>
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

class AbstractDoor
    : public TelemetryProvider,
      protected ConfigAware,
      protected Loopable<bool> {
public:
    AbstractDoor(const Config& config, SwitchHandler& openSwitch, SwitchHandler& closedSwitch)
        : ConfigAware(config)
        , openSwitch(openSwitch)
        , closedSwitch(closedSwitch) {
    }

    void begin(std::function<void(std::function<void(JsonObject&)>)> onEvent);

    /**
     * Loops the door, and returns whether the door is currently moving.
     */
    bool loop() override;

    virtual void moveTo(long position) = 0;

    void override(GateState state) {
        this->manualOverride = true;
        onEvent([](JsonObject& json) { json["manualOverride"] = true; });
        startMoving(state);
    }
    void resume() {
        this->manualOverride = false;
        onEvent([](JsonObject& json) { json["manualOverride"] = false; });
    }
    void lightChanged(float light);
    virtual void populateTelemetry(JsonObject& json) override {
        json["manualOverride"] = manualOverride;
        json["emergencyStop"] = emergencyStop;
        json["gate"] = static_cast<int>(state);
    }

protected:
    virtual void initializeMotor() = 0;
    virtual void stopMotor() = 0;
    virtual void disableMotor() = 0;

    SwitchHandler& openSwitch;
    SwitchHandler& closedSwitch;

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

    virtual bool isMoving() {
        return state == GateState::OPENING || state == GateState::CLOSING;
    }

    virtual bool continueMoving(GateState state) {
        if (millis() - movementStarted > config.movementTimeout) {
            halt("Move timed out");
            return false;
        }
        return true;
    }

    /**
     * Finishes moving in the given state.
     */
    void stopMoving(GateState state) {
        stopMotor();
        setState(state);
    }

    void halt(const String& reason) {
        Serial.println("Emergency stopping: " + reason);
        emergencyStop = true;
        stopMotor();
        disableMotor();
        onEvent([reason](JsonObject& json) {
            json["emergencyStop"] = true;
            json["reason"] = reason;
        });
    }

    unsigned long movementStarted;
};
