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
     * Maximum time in milliseconds to take to open or close the door.
     * After that time the motor will stop and the door will be disabled.
     */
    unsigned long movementTimeout;

    /**
     * Whether to invert the "gate open" switch or not.
     */
    bool invertOpenSwitch;

    /**
     * Whether to invert the "gate close" switch or not.
     */
    bool invertCloseSwitch;

    /**
     * Whether connecting WIFI is enabled.
     */
    bool wifiEnabled;

    /**
     * WIFI SSID to connect to.
     */
    String wifiSsid;

    /**
     * WIFI SSID to connect to.
     */
    String wifiPassword;

    /**
     * Whether to enable GPRS (when it's configured).
     * If not enabled, will prefer WIFI connection.
     */
    bool gprsEnabled;

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
     * Number of milliseconds between publishing state.
     */
    unsigned long statePublishingInterval;
};

class ConfigAware {
public:
    ConfigAware(const Config& config)
        : config(config) {
    }

protected:
    const Config& config;
};
