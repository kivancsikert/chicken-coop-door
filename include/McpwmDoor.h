#pragma once

#include <FastAccelStepper.h>

#include "AbstractDoor.h"

/**
 * @brief A simple implementation of AbstractDoor that uses the AccelStepper library.
 */
class McpwnDoor
    : public AbstractDoor {

public:
    McpwnDoor(const Config& config, SwitchHandler& openSwitch, SwitchHandler& closedSwitch)
        : AbstractDoor(config, openSwitch, closedSwitch)
        , motor(engine.stepperConnectToPin(MOTOR_STEP)) {
        motor->setDirectionPin(MOTOR_DIR);
        // motor->setEnablePin(MOTOR_ENABLE);
        motor->setAutoEnable(true);

        motor->setSpeedInHz(500);
        motor->setAcceleration(250);
        motor->enableOutputs();
    }

    void moveTo(long position) override {
        motor->moveTo(position);
    }

    void populateTelemetry(JsonObject& json) override {
        AbstractDoor::populateTelemetry(json);
        json["motorPosition"] = motor->getCurrentPosition();
    }

protected:

    void startMoving(GateState state) override {
        AbstractDoor::startMoving(state);
        switch (state) {
            case GateState::OPENING:
                Serial.println("Starting motor forward...");
                motor->runForward();
                break;
            case GateState::CLOSING:
                Serial.println("Starting motor backward...");
                motor->runBackward();
                break;
            default:
                Serial.printf("Started moving towards state %d which makes no sense", static_cast<int>(state));
                break;
        }
    }

    bool isMoving() override {
        return (motor->getCurrentSpeedInUs() != 0) || AbstractDoor::isMoving();
    }

    void stopMotor() override {
        Serial.println("Stopping motor...");
        motor->stopMove();
    }

    void disableMotor() override {
        // We probably don't need this with motor->setAutoEnable()
        motor->disableOutputs();
    }

private:
    FastAccelStepperEngine engine;
    FastAccelStepper* motor;
};
