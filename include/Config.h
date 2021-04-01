#pragma once

#include <ArduinoJson.h>

class Config {
public:
    void begin();
    void update(const JsonDocument& json);
    void store();

    /**
     * Light required to be above limit to open the door.
     */
    float openLightLimit;

    /**
     * Light required to be below limit to open the door.
     */
    float closeLightLimit;

    /**
     * Light sensor update interval in milliseconds.
     */
    unsigned long lightUpdateInterval;

    /**
     * The size of the moving window to abserve to smooth over light fluctuations.
     */
    unsigned long lightLatencyInterval;

    /**
     * Is moving the door enabled?
     */
    bool motorEnabled;

    /**
     * Whether to invert the "gate open" switch or not.
     */
    bool invertOpenSwitch;

    /**
     * Whether to invert the "gate close" switch or not.
     */
    bool invertCloseSwitch;

    /**
     * WIFI SSID to connect to.
     */
    String wifiSsid;

    /**
     * WIFI SSID to connect to.
     */
    String wifiPassword;

    /**
     * SIM PIN code.
     */
    String simPin;

    /**
     * GPRS APN.
     */
    String gprsApn;

    /**
     * GPRS username.
     */
    String gprsUsername;

    /**
     * GPRS password.
     */
    String gprsPassword;

    /**
     * Whether to enable GPRS (when it's configured).
     If not enabled, will prefer WIFI connection.
     */
    bool gprsEnable;

    /**
     * Number of milliseconds between publishing state.
     */
    unsigned long statePublishingInterval;
};

class ConfigAware {
public:
    ConfigAware(Config& config)
        : config(config) {
    }

protected:
    Config& config;
};
