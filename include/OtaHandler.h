#pragma once

#include <Arduino.h>
#include <ArduinoOTA.h>

class OtaHandler {
public:
    OtaHandler();
    void begin(const char* hostname);
    void loop();
};
