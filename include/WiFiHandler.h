#pragma once

#include "Config.h"
#include "WiFi.h"

class WiFiHandler {
public:
    WiFiHandler(Config& config);
    void begin(const String& hostname);
    Client& getClient() {
        return client;
    }

private:
    void startWifi();
    bool awaitConnect();

    Config& config;
    WiFiClient client;
};
