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
    bool ensureConnected();

    bool connected() {
        return WiFi.status() == WL_CONNECTED;
    }

    WiFiClientSecure& getClient() {
        return client;
    }

private:
    bool connect();
    bool awaitConnect();

    const String hostname;
    const String caCert;
    WiFiClientSecure client;
};
