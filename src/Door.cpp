#include "Door.h"

#define STEPS_AT_ONCE 100

void Door::begin() {
    motor.setMaxSpeed(500);
    motor.setSpeed(500);
    motor.setAcceleration(500);
    Serial.println("Motor configured");

    light.setOnUpdate([this](float currentLight) {
        if (currentLight < config.closeLightLimit && state != GateState::CLOSED && state != GateState::CLOSING) {
            Serial.println("Closing...");
            startMoving(GateState::CLOSING);
        } else if (currentLight > config.openLightLimit && state != GateState::OPEN && state != GateState::OPENING) {
            Serial.println("Opening...");
            startMoving(GateState::OPENING);
        }
    });
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
        if (closedSwitch.getState()) {
            Serial.println("Closed");
            motor.stop();
            state = GateState::CLOSED;
        } else {
            advanceMotor(-STEPS_AT_ONCE);
        }
    } else if (state == GateState::OPENING) {
        if (openSwitch.getState()) {
            Serial.println("Open");
            motor.stop();
            state = GateState::OPEN;
        } else {
            advanceMotor(STEPS_AT_ONCE);
        }
    }
    return true;
}

void Door::startMoving(GateState state) {
    this->state = state;
    movementStarted = millis();
}

void Door::advanceMotor(long steps) {
    if (millis() - movementStarted > config.movementTimeout) {
        Serial.println("Move timed out, emergency stopping");
        emergencyStop = true;
    }
    motor.move(steps);
}

void Door::populateTelemetry(JsonDocument& json) {
    json["emergencyStop"] = emergencyStop;
    json["light"] = light.getCurrentLevel();
    json["gate"] = static_cast<int>(state);
    json["openSwitch"] = openSwitch.getState();
    json["closedSwitch"] = closedSwitch.getState();
    json["motorPosition"] = motor.currentPosition();
}
