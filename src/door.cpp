#include "door.h"

#define STEPS_AT_ONCE 100

AccelStepper motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

BH1750 lightMeter;

Door::Door(Config& config, MqttHandler& mqttHandler)
    : config(config)
    , mqttHandler(mqttHandler) {
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
    unsigned long currentMillis = millis();
    updateLight(currentMillis);
    updateMotor();
    publishTelemetry(currentMillis);
}

void Door::updateLight(unsigned long currentMillis) {
    if (currentMillis - previousLightUpdateMillis > config.lightUpdateInterval) {
        previousLightUpdateMillis = currentMillis;

        currentLight = lightMeter.readLightLevel();
        if (currentLight < config.closeLightLimit && gateState == GateState::OPEN) {
            Serial.println("Closing...");
            gateState = GateState::CLOSING;
        } else if (currentLight > config.openLightLimit && gateState == GateState::CLOSED) {
            Serial.println("Opening...");
            gateState = GateState::OPENING;
        }
    }
}

void Door::updateMotor() {
    openSwitch = digitalRead(OPEN_PIN) ^ config.invertOpenSwitch;
    closedSwitch = digitalRead(CLOSED_PIN) ^ config.invertCloseSwitch;

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
}

void Door::executeCommand(const JsonDocument& json) {
    if (json.containsKey("moveTo")) {
        long targetPosition = json["moveTo"];
        Serial.println("Moving door to " + String(targetPosition));
        motor.moveTo(targetPosition);
    }
}

void Door::publishTelemetry(unsigned long currentMillis) {
    if (currentMillis - previousStatePublishMillis > config.statePublishingInterval) {
        previousStatePublishMillis = currentMillis;

        DynamicJsonDocument json(2048);
        json["light"] = currentLight;
        json["gate"] = static_cast<int>(gateState);
        json["openSwitch"] = openSwitch;
        json["closedSwitch"] = closedSwitch;
        json["motorPosition"] = motor.currentPosition();
        mqttHandler.publishTelemetry(json);
        Serial.println("Published state");
    }
}
