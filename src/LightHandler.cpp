#include "LightHandler.h"

void LightHandler::begin(int sda, int scl) {
    Wire.begin(sda, scl);
    if (sensor.begin(BH1750::CONTINUOUS_LOW_RES_MODE)) {
        Serial.println("Light sensor initialised");
    } else {
        Serial.println("Error initialising light sensor");
    }
}

void LightHandler::timedLoop() {
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
