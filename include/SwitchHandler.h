#pragma once

#include "Loopable.h"
#include <functional>

class SwitchHandler
    : public TimedLoopable {
public:
    SwitchHandler(int pin, std::function<bool()> invertSwitch)
        : pin(pin)
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

    bool getState() {
        return state;
    }

private:
    const int pin;
    const std::function<bool()> invertSwitch;

    bool state;
};
