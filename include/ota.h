#include <Arduino.h>
#include <ArduinoOTA.h>

class Ota {
public:
    Ota();
    void begin(const char* hostname);
    void handle();
};
