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

class CompositeTelemetryProvider : public TelemetryProvider {
public:
    CompositeTelemetryProvider(std::list<TelemetryProvider*> delegates)
        : delegates(delegates) {
    }

    void populateTelemetry(JsonObject& json) override {
        json["uptime"] = millis();
        for (auto& delegate : delegates) {
            delegate->populateTelemetry(json);
        }
    }

private:
    std::list<TelemetryProvider*> delegates;
};

class TelemetryPublisher
    : public TimedLoopable<void>,
      private ConfigAware {
public:
    TelemetryPublisher(
        const Config& config,
        MqttHandler& mqtt,
        const String& topic,
        TelemetryProvider& telemetryProvider)
        : ConfigAware(config)
        , mqtt(mqtt)
        , topic(topic)
        , telemetryProvider(telemetryProvider) {
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
        telemetryProvider.populateTelemetry(root);
        mqtt.publish(topic, doc);
    }
    void defaultValue() override {
    }

private:
    MqttHandler& mqtt;
    const String topic;
    TelemetryProvider& telemetryProvider;
};
