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
        if (currentLight < config.closeLightLimit && state == GateState::OPEN) {
            Serial.println("Closing...");
            state = GateState::CLOSING;
        } else if (currentLight > config.openLightLimit && state == GateState::CLOSED) {
            Serial.println("Opening...");
            state = GateState::OPENING;
        }
    });
}

bool Door::loop() {
    unsigned long currentMillis = millis();
    bool moving = updateMotor();
    if (!moving) {
        publishTelemetry(currentMillis);
    }
    return moving;
}

bool Door::updateMotor() {
    openSwitch = digitalRead(OPEN_PIN) ^ config.invertOpenSwitch;
    closedSwitch = digitalRead(CLOSED_PIN) ^ config.invertCloseSwitch;

    if (!motor.run()) {
        if (state == GateState::OPEN || state == GateState::CLOSED) {
            motor.disableOutputs();
            delay(250);
            return false;
        }
    }

    if (state == GateState::CLOSING) {
        if (closedSwitch) {
            Serial.println("Closed");
            motor.stop();
            state = GateState::CLOSED;
        } else {
            motor.move(-STEPS_AT_ONCE);
        }
    } else if (state == GateState::OPENING) {
        if (openSwitch) {
            Serial.println("Open");
            motor.stop();
            state = GateState::OPEN;
        } else {
            motor.move(STEPS_AT_ONCE);
        }
    }
    return true;
}

void Door::publishTelemetry(unsigned long currentMillis) {
    if (currentMillis - previousStatePublishMillis > config.statePublishingInterval) {
        previousStatePublishMillis = currentMillis;

        DynamicJsonDocument json(2048);
        json["light"] = light.getCurrentLevel();
        json["gate"] = static_cast<int>(state);
        json["openSwitch"] = openSwitch;
        json["closedSwitch"] = closedSwitch;
        json["motorPosition"] = motor.currentPosition();
        mqtt.publishTelemetry(json);
    }
}
