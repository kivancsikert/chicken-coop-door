#pragma once

#include <ArduinoJson.h>
#include <Client.h>
#include <MQTT.h>
#include <functional>

#include "Loopable.h"
#include "NtpHandler.h"
#include "WiFiHandler.h"

#define MQTT_BUFFER_SIZE 2048

// Time (seconds) to expire token += 20 minutes for drift
// Maximum 24H (3600 * 24)
#define JWT_EXPIRATION_IN_SECONDS (60 * 60)

class MqttHandler : Loopable<void> {
public:
    MqttHandler(WiFiHandler& wifiHandler, NtpHandler& ntpHandler);

    void begin(
        const JsonDocument& mqttConfig,
        std::function<void(const JsonDocument&)> onConfigChange,
        std::function<void(const JsonDocument&)> onCommand);
    void loop() override;

    bool publishStatus(const JsonDocument& json);
    bool publishTelemetry(const JsonDocument& json);

private:
    bool publish(const String& topic, const JsonDocument& json);
    bool subscribe(const String& topic, int qos);

    String clientId;
    String prefix;

    WiFiHandler& wifiHandler;
    NtpHandler& ntpHandler;
    MQTTClient mqttClient;

    // See https://cloud.google.com/iot/docs/how-tos/exponential-backoff
    int __backoff__ = 1000;    // current backoff, milliseconds
    static const int __factor__ = 2.5f;
    static const int __minbackoff__ = 1000;      // minimum backoff, ms
    static const int __max_backoff__ = 60000;    // maximum backoff, ms
    static const int __jitter__ = 500;           // max random jitter, ms
};
