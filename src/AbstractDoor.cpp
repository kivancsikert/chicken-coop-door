#include "Door.h"

void AbstractDoor::begin(std::function<void(std::function<void(JsonObject&)>)> onEvent) {
    this->onEvent = onEvent;

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
    Serial.println(message);

    // Publish initial state
    onEvent([message](JsonObject& json) {
        json["init"] = true;
        json["message"] = message;
    });
}

void AbstractDoor::lightChanged(float light) {
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

bool AbstractDoor::loop() {
    bool movementExpected = !emergencyStop
        && config.motorEnabled
        && isMoving();

    if (!movementExpected) {
        disableMotor();
        return false;
    }

    if (state == GateState::CLOSING) {
        if (closedSwitch.isEngaged()) {
            Serial.println("Closed");
            stopMoving(GateState::CLOSED);
        } else {
            continueMoving(GateState::CLOSING);
        }
    } else if (state == GateState::OPENING) {
        if (openSwitch.isEngaged()) {
            Serial.println("Open");
            stopMoving(GateState::OPEN);
        } else {
            continueMoving(GateState::OPENING);
        }
    }
    return true;
}
