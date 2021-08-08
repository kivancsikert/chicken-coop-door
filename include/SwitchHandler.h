#pragma once

#include "Loopable.h"
#include <FunctionalInterrupt.h>
#include <functional>

class SwitchHandler
    : public TelemetryProvider {
public:
    SwitchHandler(const String& name, int pin, std::function<bool()> invertSwitch)
        : name(name)
        , pin(pin)
        , invertSwitch(invertSwitch) {
        pinMode(pin, INPUT_PULLUP);
    }

    void begin() {
        state = digitalRead(pin);
        attachInterrupt(digitalPinToInterrupt(pin), [this]() {
            state = true;
        }, RISING);
        attachInterrupt(digitalPinToInterrupt(pin), [this]() {
            state = false;
        }, FALLING);
    }

    void populateTelemetry(JsonObject& json) override {
        json[name] = isEngaged();
    }

    bool isEngaged() {
        return state != invertSwitch();
    }

private:
    const String name;
    const int pin;
    const std::function<bool()> invertSwitch;

    volatile bool state;
};
