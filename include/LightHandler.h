#pragma once

#include "Config.h"
#include "Loopable.h"
#include <BH1750.h>
#include <Wire.h>
#include <functional>

class LightHandler
    : public TimedLoopable {
public:
    LightHandler(Config& config)
        : config(config) {
    }

    void begin(int sda, int scl);

    void setOnUpdate(std::function<void(float)> onUpdate) {
        this->onUpdate = onUpdate;
    }

    float getCurrentLevel() {
        return currentLevel;
    }

protected:
    unsigned long getPeriodInMillis() override {
        return config.lightUpdateInterval;
    }
    void timedLoop() override;

private:
    Config& config;
    BH1750 sensor;

    std::function<void(float)> onUpdate;

    float currentLevel = 0;
};
