#pragma once

#include "Config.h"
#include "Loopable.h"
#include "MqttHandler.h"
#include <ArduinoJson.h>
#include <list>

class TelemetryProvider {
public:
    virtual void populateTelemetry(JsonObject& json) = 0;
};

class TelemetryPublisher
    : public TimedLoopable,
      private ConfigAware {
public:
    TelemetryPublisher(const Config& config, MqttHandler& mqtt, std::list<TelemetryProvider*> providers)
        : ConfigAware(config)
        , mqtt(mqtt)
        , providers(providers) {
    }

    void begin() {
    }

protected:
    unsigned long getPeriodInMillis() override {
        // TODO Rename this to telemetry publishing interval
        return config.statePublishingInterval;
    }

    void timedLoop() override {
        DynamicJsonDocument doc(2048);
        JsonObject root = doc.to<JsonObject>();
        for (auto& provider : providers) {
            provider->populateTelemetry(root);
        }
        mqtt.publishTelemetry(doc);
    }

private:
    MqttHandler& mqtt;
    std::list<TelemetryProvider*> providers;
};
