#include "door.h"

AccelStepper motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

BH1750 lightMeter;

Door::Door(Config& config)
    : config(config) {
}

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
    openSwitch = digitalRead(OPEN_PIN) ^ config.invertOpenSwitch;
    closedSwitch = digitalRead(CLOSED_PIN) ^ config.invertCloseSwitch;

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        currentLight = lightMeter.readLightLevel();
        if (currentLight < config.closeLightLimit && gateState == GateState::OPEN) {
            Serial.println("Closing...");
            gateState = GateState::CLOSING;
        } else if (currentLight > config.openLightLimit && gateState == GateState::CLOSED) {
            Serial.println("Opening...");
            gateState = GateState::OPENING;
        }
    }

    if (!motor.run()) {
        if (gateState == GateState::OPEN || gateState == GateState::CLOSED) {
            motor.disableOutputs();
            delay(250);
            return;
        }
    }

    if (gateState == GateState::CLOSING) {
        if (closedSwitch) {
            Serial.println("Closed");
            motor.stop();
            gateState = GateState::CLOSED;
        } else {
            motor.move(stepsAtOnce);
        }
    } else if (gateState == GateState::OPENING) {
        if (openSwitch) {
            Serial.println("Open");
            motor.stop();
            gateState = GateState::OPEN;
        } else {
            motor.move(-stepsAtOnce);
        }
    }
}
