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
    onUpdate(currentLevel);
}
