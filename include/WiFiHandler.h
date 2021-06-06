#pragma once

#include "Config.h"
#include <WiFiClientSecure.h>

class WiFiHandler
    : private ConfigAware {
public:
    WiFiHandler(const Config& config, const String& hostname, const String& caCert)
        : ConfigAware(config)
        , hostname(hostname)
        , caCert(caCert) {
    }

    void begin();

    WiFiClientSecure& getClient() {
        return client;
    }

private:
    void startWifi();
    bool awaitConnect();

    const String hostname;
    const String caCert;
    WiFiClientSecure client;
};
