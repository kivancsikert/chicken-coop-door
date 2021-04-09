#pragma once

#include "Loopable.h"
#include <functional>

class SwitchHandler
    : public TimedLoopable,
      public TelemetryProvider {
public:
    SwitchHandler(const String& name, int pin, std::function<bool()> invertSwitch)
        : name(name)
        , pin(pin)
        , invertSwitch(invertSwitch) {
    }

    void begin() {
        pinMode(pin, INPUT_PULLUP);
    }

    virtual void timedLoop() override {
        state = digitalRead(pin) ^ invertSwitch();
    }

    unsigned long getPeriodInMillis() override {
        // TODO Make this configurable
        return 100;
    }

    void populateTelemetry(JsonObject& json) override {
        json[name] = state;
    }

    bool getState() {
        return state;
    }

private:
    const String name;
    const int pin;
    const std::function<bool()> invertSwitch;

    bool state;
};
