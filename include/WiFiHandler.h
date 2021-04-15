#pragma once

#include "Config.h"
#include <WiFiClientSecure.h>

class WiFiHandler
    : private ConfigAware {
public:
    WiFiHandler(const Config& config, const String& caCert)
        : ConfigAware(config)
#if defined(ESP32)
        , caCert(caCert)
#elif defined(ESP8266)
        , caCert(new BearSSL::X509List(caCert.c_str()))
#endif
    {
    }

    void begin(const String& hostname);

    Client& getClient() {
        return client;
    }

private:
    void startWifi();
    bool awaitConnect();

    WiFiClientSecure client;
#if defined(ESP32)
    const String& caCert;
#elif defined(ESP8266)
    BearSSL::X509List* caCert;
#endif
};
