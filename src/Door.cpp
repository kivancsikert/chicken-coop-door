#include "Door.h"

#define STEPS_AT_ONCE 100

void Door::begin() {
    pinMode(OPEN_PIN, INPUT_PULLUP);
    pinMode(CLOSED_PIN, INPUT_PULLUP);

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
    unsigned long currentMillis = millis();
    bool moving = updateMotor(currentMillis);
    if (!moving) {
        publishTelemetry(currentMillis);
    }
    return moving;
}

bool Door::updateMotor(unsigned long currentMillis) {
    openSwitch = digitalRead(OPEN_PIN) ^ config.invertOpenSwitch;
    closedSwitch = digitalRead(CLOSED_PIN) ^ config.invertCloseSwitch;

    bool movementExpected = !emergencyStop
        && config.motorEnabled
        && (motor.run() || state == GateState::OPENING || state == GateState::CLOSING);

    if (!movementExpected) {
        motor.disableOutputs();
        delay(250);
        return false;
    }

    if (state == GateState::CLOSING) {
        if (closedSwitch) {
            Serial.println("Closed");
            motor.stop();
            state = GateState::CLOSED;
        } else {
            advanceMotor(currentMillis, -STEPS_AT_ONCE);
        }
    } else if (state == GateState::OPENING) {
        if (openSwitch) {
            Serial.println("Open");
            motor.stop();
            state = GateState::OPEN;
        } else {
            advanceMotor(currentMillis, STEPS_AT_ONCE);
        }
    }
    return true;
}

void Door::startMoving(GateState state) {
    this->state = state;
    movementStarted = millis();
}

void Door::advanceMotor(unsigned long currentMillis, long steps) {
    if (currentMillis - movementStarted > config.movementTimeout) {
        Serial.println("Move timed out, emergency stopping");
        emergencyStop = true;
    }
    motor.move(steps);
}

void Door::publishTelemetry(unsigned long currentMillis) {
    if (currentMillis - previousStatePublishMillis > config.statePublishingInterval) {
        previousStatePublishMillis = currentMillis;

        DynamicJsonDocument json(2048);
        json["emergencyStop"] = emergencyStop;
        json["light"] = light.getCurrentLevel();
        json["gate"] = static_cast<int>(state);
        json["openSwitch"] = openSwitch;
        json["closedSwitch"] = closedSwitch;
        json["motorPosition"] = motor.currentPosition();
        mqtt.publishTelemetry(json);
    }
}
