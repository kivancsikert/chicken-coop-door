#include "Door.h"

#define STEPS_AT_ONCE 500

void Door::begin(std::function<void(std::function<void(JsonObject&)>)> onEvent) {
    this->onEvent = onEvent;

    motor.setMaxSpeed(500);
    motor.setSpeed(500);
    motor.setAcceleration(500);
    Serial.println("Motor configured");

    // Set initial state
    String message;
    if (openSwitch.isEngaged()) {
        if (closedSwitch.isEngaged()) {
            halt("Both open and close switches are engaged");
            return;
        }
        message = "Door initialized in open state";
        state = GateState::OPEN;
    } else if (closedSwitch.isEngaged()) {
        message = "Door initialized in closed state";
        state = GateState::CLOSED;
    } else {
        message = "Door initialized; switches not engaged, closing";
        state = GateState::CLOSING;
    }

    // Publish initial state
    onEvent([message](JsonObject& json) {
        json["init"] = true;
        json["message"] = message;
    });
}

void Door::lightChanged(float light) {
    // Ignore light changes when in manual override mode
    if (manualOverride) {
        return;
    }
    if (light < config.closeLightLimit && state != GateState::CLOSED && state != GateState::CLOSING) {
        Serial.println("Closing...");
        startMoving(GateState::CLOSING);
    } else if (light > config.openLightLimit && state != GateState::OPEN && state != GateState::OPENING) {
        Serial.println("Opening...");
        startMoving(GateState::OPENING);
    }
}

bool Door::loop() {
    bool movementExpected = !emergencyStop
        && config.motorEnabled
        && (motor.run() || state == GateState::OPENING || state == GateState::CLOSING);

    if (!movementExpected) {
        motor.disableOutputs();
        delay(250);
        return false;
    }

    if (state == GateState::CLOSING) {
        if (closedSwitch.isEngaged()) {
            Serial.println("Closed");
            stopMoving(GateState::CLOSED);
        } else {
            advanceMotor(-STEPS_AT_ONCE);
        }
    } else if (state == GateState::OPENING) {
        if (openSwitch.isEngaged()) {
            Serial.println("Open");
            stopMoving(GateState::OPEN);
        } else {
            advanceMotor(STEPS_AT_ONCE);
        }
    }
    return true;
}

void Door::advanceMotor(long steps) {
    if (millis() - movementStarted > config.movementTimeout) {
        halt("Move timed out");
        return;
    }
    motor.move(steps);
}
