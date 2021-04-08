#pragma once

#include "Config.h"
#include "WiFi.h"

class WiFiHandler
    : private ConfigAware {
public:
    WiFiHandler(const Config& config)
        : ConfigAware(config) {
    }

    void begin(const String& hostname);

    Client& getClient() {
        return client;
    }

private:
    void startWifi();
    bool awaitConnect();

    WiFiClient client;
};
