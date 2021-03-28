#include "door.h"

const int stepsPerRevolution = 2048;

AccelStepper motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

BH1750 lightMeter;

void Door::begin() {
    pinMode(OPEN_PIN, INPUT_PULLUP);
    pinMode(CLOSED_PIN, INPUT_PULLUP);

    motor.setMaxSpeed(500);
    motor.setSpeed(500);
    motor.setAcceleration(500);
    Serial.println("Motor configured");

    Wire.begin(LIGHT_SDA, LIGHT_SCL);
    if (lightMeter.begin()) {
        Serial.println("Light sensor initialised");
    } else {
        Serial.println("Error initialising light sensor");
    }
}

void Door::loop() {
    state.openSwitch = digitalRead(OPEN_PIN) ^ config.invertOpenSwitch;
    state.closedSwitch = digitalRead(CLOSED_PIN) ^ config.invertCloseSwitch;

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        state.currentLight = lightMeter.readLightLevel();
        if (state.currentLight < config.closeLightLimit && state.gateState == GateState::OPEN) {
            Serial.println("Closing...");
            state.gateState = GateState::CLOSING;
        } else if (state.currentLight > config.openLightLimit && state.gateState == GateState::CLOSED) {
            Serial.println("Opening...");
            state.gateState = GateState::OPENING;
        }
    }

    if (!motor.run()) {
        if (state.gateState == GateState::OPEN || state.gateState == GateState::CLOSED) {
            motor.disableOutputs();
            delay(250);
            return;
        }
    }

    if (state.gateState == GateState::CLOSING) {
        if (state.closedSwitch) {
            Serial.println("Closed");
            motor.stop();
            state.gateState = GateState::CLOSED;
        } else {
            motor.move(stepsAtOnce);
        }
    } else if (state.gateState == GateState::OPENING) {
        if (state.openSwitch) {
            Serial.println("Open");
            motor.stop();
            state.gateState = GateState::OPEN;
        } else {
            motor.move(-stepsAtOnce);
        }
    }
}
