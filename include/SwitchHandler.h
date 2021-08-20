#pragma once

#include "Telemetry.h"

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
    }

    void populateTelemetry(JsonObject& json) override {
        json[name] = isEngaged();
    }

    bool isEngaged() {
        int state = digitalRead(pin);
        return state != invertSwitch();
    }

private:
    const String name;
    const int pin;
    const std::function<bool()> invertSwitch;
};
