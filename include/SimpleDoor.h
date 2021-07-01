#pragma once

#include <AccelStepper.h>

#include "AbstractDoor.h"

#define STEPS_AT_ONCE 500

/**
 * @brief A simple implementation of AbstractDoor that uses the AccelStepper library.
 */
class SimpleDoor
    : public AbstractDoor {

public:
    SimpleDoor(const Config& config, SwitchHandler& openSwitch, SwitchHandler& closedSwitch)
        : AbstractDoor(config, openSwitch, closedSwitch)
        , motor(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4) {
    }

    void moveTo(long position) override {
        motor.moveTo(position);
    }

    void populateTelemetry(JsonObject& json) override {
        AbstractDoor::populateTelemetry(json);
        json["motorPosition"] = motor.currentPosition();
    }

protected:
    void initializeMotor() override {
        motor.setMaxSpeed(400);
        motor.setSpeed(400);
        motor.setAcceleration(100);
    }

    bool isMoving() override {
        return motor.run() || AbstractDoor::isMoving();
    }

    bool continueMoving(GateState state) override {
        if (!AbstractDoor::continueMoving(state)) {
            return false;
        }
        int steps;
        switch (state) {
            case GateState::OPENING:
                steps = STEPS_AT_ONCE;
                break;
            case GateState::CLOSING:
                steps = -STEPS_AT_ONCE;
                break;
            default:
                return false;
        }
        motor.move(steps);
        return true;
    }

    void stopMotor() override {
        motor.stop();
    }

    void disableMotor() override {
        motor.disableOutputs();
    }

private:
    /**
     * Advances the motor in the given direction.
     */
    void advanceMotor(long steps);

    AccelStepper motor;
};
