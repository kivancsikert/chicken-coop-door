#pragma once

#include "Config.h"
#include "Loopable.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>

class WiFiHandler
    : public TimedLoopable,
      private ConfigAware {
public:
    WiFiHandler(const Config& config, const String& hostname, const String& caCert)
        : ConfigAware(config)
        , hostname(hostname)
        , caCert(caCert) {
    }

    void begin();

    bool connected() {
        return WiFi.status() == WL_CONNECTED;
    }

    WiFiClientSecure& getClient() {
        return client;
    }

protected:
    unsigned long getPeriodInMillis() override {
        return 500;
    }
    void timedLoop() override;

private:
    void startConnecting();
    void stopConnecting();
    bool awaitConnect();

    const String hostname;
    const String caCert;
    WiFiClientSecure client;

    bool connecting = false;
    unsigned long connectionStarted;
};
