#pragma once

#include "Loopable.h"
#include <functional>

class SwitchHandler
    : public TimedLoopable<void>,
      public TelemetryProvider {
public:
    SwitchHandler(const String& name, int pin, std::function<bool()> invertSwitch)
        : name(name)
        , pin(pin)
        , invertSwitch(invertSwitch) {
        pinMode(pin, INPUT_PULLUP);
    }

    void begin() {
        update();
    }

    void timedLoop() override {
        update();
    }
    void defaultValue() override {}

    void update() {
        engaged = digitalRead(pin) ^ invertSwitch();
    }

    unsigned long getPeriodInMillis() override {
        // TODO Make this configurable
        return 100;
    }

    void populateTelemetry(JsonObject& json) override {
        json[name] = engaged;
    }

    bool isEngaged() {
        return engaged;
    }

private:
    const String name;
    const int pin;
    const std::function<bool()> invertSwitch;

    bool engaged;
};
