#pragma once

#include <ArduinoJson.h>
#include <CircularBuffer.h>
#include <Client.h>
#include <MQTT.h>
#include <functional>

#include "Loopable.h"
#include "NtpHandler.h"
#include "WiFiHandler.h"

#define MQTT_BUFFER_SIZE 2048
#define MQTT_QUEUED_MESSAGES_MAX 16

// Time (seconds) to expire token += 20 minutes for drift
// Maximum 24H (3600 * 24)
#define JWT_EXPIRATION_IN_SECONDS (60 * 60)

struct MqttMessage {
    MqttMessage()
        : topic("")
        , payload("")
        , retain(false)
        , qos(0) {
    }

    MqttMessage(const String& topic, const JsonDocument& payload, boolean retain, int qos)
        : topic(topic)
        , retain(retain)
        , qos(qos) {
        serializeJson(payload, this->payload);
    }

    String topic;
    String payload;
    boolean retain;
    int qos;
};

class MqttHandler
    : public TimedLoopable<void> {
public:
    MqttHandler(WiFiHandler& wifiHandler, NtpHandler& ntpHandler);

    void begin(
        const JsonDocument& mqttConfig,
        std::function<void(const JsonDocument&)> onConfigChange,
        std::function<void(const JsonDocument&)> onCommand);

    bool publish(const String& topic, const JsonDocument& json, bool retained = false, int qos = 0);
    bool subscribe(const String& topic, int qos);

protected:
    void timedLoop() override;
    unsigned long getPeriodInMillis() override {
        return 500;
    }
    void defaultValue() override {
    }

private:
    bool tryConnect();

    String host;
    int port;
    String clientId;
    String prefix;

    WiFiHandler& wifiHandler;
    NtpHandler& ntpHandler;
    MQTTClient mqttClient;

    bool connecting = false;
    unsigned long connectionStarted;

    CircularBuffer<MqttMessage, MQTT_QUEUED_MESSAGES_MAX> publishQueue;

    // See https://cloud.google.com/iot/docs/how-tos/exponential-backoff
    int __backoff__ = 1000;    // current backoff, milliseconds
    static const int __factor__ = 2.5f;
    static const int __minbackoff__ = 1000;      // minimum backoff, ms
    static const int __max_backoff__ = 60000;    // maximum backoff, ms
    static const int __jitter__ = 500;           // max random jitter, ms
};
