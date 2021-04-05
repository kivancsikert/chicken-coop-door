#pragma once

#include "Config.h"
#include "Loopable.h"
#include <BH1750.h>
#include <Wire.h>
#include <functional>
#include <deque>

class LightHandler
    : public TimedLoopable, ConfigAware {
public:
    LightHandler(const Config& config)
        : ConfigAware(config) {
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
    BH1750 sensor;

    std::deque<float> measurements;
    double sum;
    std::function<void(float)> onUpdate;

    float currentLevel = 0;
};
