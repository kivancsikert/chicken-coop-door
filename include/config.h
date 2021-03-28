#pragma once

#include <ArduinoJson.h>

class Config {
public:
    void load(const JsonDocument& json);
    void store(JsonDocument& json);

    /**
     * Light required to be above limit to open the door.
     */
    float openLightLimit = 120;

    /**
     * Light required to be below limit to open the door.
     */
    float closeLightLimit = 50;

    /**
     * Whether to invert the "gate open" switch or not.
     */
    bool invertOpenSwitch = true;

    /**
     * Whether to invert the "gate close" switch or not.
     */
    bool invertCloseSwitch = true;

    /**
     * WIFI SSID to connect to.
     */
    String wifiSsid = "";

    /**
     * WIFI SSID to connect to.
     */
    String wifiPassword = "";
};
