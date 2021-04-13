#pragma once

#include "Config.h"
#include <WiFiClientSecure.h>

class WiFiHandler
    : private ConfigAware {
public:
    WiFiHandler(const Config& config)
        : ConfigAware(config) {
    }

    void begin(const String& hostname, const String& caCert);

    Client& getClient() {
        return client;
    }

private:
    void startWifi();
    bool awaitConnect();

    WiFiClientSecure client;
};
