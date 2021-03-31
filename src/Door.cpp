#include "Door.h"

#define STEPS_AT_ONCE 100

AccelStepper motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

void Door::begin() {
    pinMode(OPEN_PIN, INPUT_PULLUP);
    pinMode(CLOSED_PIN, INPUT_PULLUP);

    motor.setMaxSpeed(500);
    motor.setSpeed(500);
    motor.setAcceleration(500);
    Serial.println("Motor configured");

    light.setOnUpdate([this](float currentLight) {
        if (currentLight < config.closeLightLimit && gateState == GateState::OPEN) {
            Serial.println("Closing...");
            gateState = GateState::CLOSING;
        } else if (currentLight > config.openLightLimit && gateState == GateState::CLOSED) {
            Serial.println("Opening...");
            gateState = GateState::OPENING;
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
        if (gateState == GateState::OPEN || gateState == GateState::CLOSED) {
            motor.disableOutputs();
            delay(250);
            return false;
        }
    }

    if (gateState == GateState::CLOSING) {
        if (closedSwitch) {
            Serial.println("Closed");
            motor.stop();
            gateState = GateState::CLOSED;
        } else {
            motor.move(-STEPS_AT_ONCE);
        }
    } else if (gateState == GateState::OPENING) {
        if (openSwitch) {
            Serial.println("Open");
            motor.stop();
            gateState = GateState::OPEN;
        } else {
            motor.move(STEPS_AT_ONCE);
        }
    }
    return true;
}

void Door::moveTo(long position) {
    motor.moveTo(position);
}

void Door::publishTelemetry(unsigned long currentMillis) {
    if (currentMillis - previousStatePublishMillis > config.statePublishingInterval) {
        previousStatePublishMillis = currentMillis;

        DynamicJsonDocument json(2048);
        json["light"] = light.getCurrentLevel();
        json["gate"] = static_cast<int>(gateState);
        json["openSwitch"] = openSwitch;
        json["closedSwitch"] = closedSwitch;
        json["motorPosition"] = motor.currentPosition();
        mqtt.publishTelemetry(json);
    }
}
