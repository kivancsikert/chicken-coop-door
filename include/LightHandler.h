#pragma once

#include "Config.h"
#include "Loopable.h"
#include "Telemetry.h"
#include <BH1750.h>
#include <Wire.h>
#include <deque>
#include <functional>

class LightHandler
    : public TimedLoopable<void>,
      public TelemetryProvider,
      private ConfigAware {
public:
    LightHandler(const Config& config)
        : ConfigAware(config) {
    }

    void begin(int sda, int scl) {
        Serial.printf("Initializing I2C for light sensor, SDA = %d, SCL = %d\n", sda, scl);
        Wire.begin(sda, scl);
        if (sensor.begin(BH1750::CONTINUOUS_LOW_RES_MODE)) {
            Serial.println("Light sensor initialised");
        } else {
            Serial.println("Error initialising light sensor");
        }
    }

    void setOnUpdate(std::function<void(float)> onUpdate) {
        this->onUpdate = onUpdate;
    }

    void populateTelemetry(JsonObject& json) override {
        json["light"] = currentLevel;
    }

protected:
    unsigned long getPeriodInMillis() override {
        return config.lightUpdateInterval;
    }
    void timedLoop() override {
        currentLevel = sensor.readLightLevel();

        size_t maxMaxmeasurements = config.lightLatencyInterval / config.lightUpdateInterval;
        while (measurements.size() >= maxMaxmeasurements) {
            sum -= measurements.front();
            measurements.pop_front();
        }
        measurements.emplace_back(currentLevel);
        sum += currentLevel;

        double averageLevel = sum / measurements.size();
        onUpdate(averageLevel);
    }
    void defaultValue() override {}

private:
    BH1750 sensor;

    std::deque<float> measurements;
    double sum;
    std::function<void(float)> onUpdate;

    float currentLevel = 0;
};
