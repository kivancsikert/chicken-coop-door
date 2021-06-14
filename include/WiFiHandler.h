#pragma once

#include "Config.h"
#include "Loopable.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>

class WiFiHandler
    : public TimedLoopable<bool>,
      private ConfigAware {
public:
    WiFiHandler(const Config& config, const String& hostname)
        : ConfigAware(config)
        , hostname(hostname) {
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
    bool timedLoop() override;
    bool defaultValue() override {
        return false;
    }

private:
    void startConnecting();
    void stopConnecting();
    bool awaitConnect();

    const String hostname;
    WiFiClientSecure client;

    bool connecting = false;
    unsigned long connectionStarted;
};
