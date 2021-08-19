#include "MqttHandler.h"

MqttHandler::MqttHandler(WiFiHandler& wifiHandler, NtpHandler& ntpHandler)
    : wifiHandler(wifiHandler)
    , ntpHandler(ntpHandler) {
}

void MqttHandler::begin(
    const JsonDocument& mqttConfig,
    std::function<void(JsonDocument&)> onConfigChange,
    std::function<void(JsonDocument&)> onCommand) {
    Serial.println("Initializing MQTT connector...");
    onConfigChange = onConfigChange;
    onCommand = onCommand;

    host = mqttConfig["host"].as<String>();
    port = mqttConfig["port"].as<int>();
    clientId = mqttConfig["clientId"].as<String>();
    prefix = mqttConfig["prefix"].as<String>();
}

void MqttHandler::loop() {
    if (mqttClient == nullptr) {
        if (!wifiHandler.connected()) {
            Serial.println("Couldn't connect to MQTT because WIFI is down");
            return;
        }

        if (!ntpHandler.isUpToDate()) {
            Serial.println("Couldn't connect to MQTT because NTP is not up to date");
            return;
        }

        Serial.printf("Configuring MQTT at '%s:%d'...\n", host.c_str(), port);
        mqttClient = new MQTTClient(MQTT_BUFFER_SIZE);
        mqttClient->setOptions(
            180,     // keepAlive
            true,    // cleanSession
            10000    // timeout
        );
        mqttClient->begin(host.c_str(), port, wifiHandler.getClient());
        mqttClient->onMessage([this](const String& topic, const String& payload) {
#ifdef DUMP_MQTT
            Serial.println("Received '" + topic + "' (size: " + payload.length() + "): " + payload);
#endif
            DynamicJsonDocument json(payload.length() * 2);
            deserializeJson(json, payload);
            if (topic.endsWith("/config")) {
                onConfigChange(json);
            } else if (topic.endsWith("/commands")) {
                onCommand(json);
            }
        });
    }

    if (!mqttClient->connected()) {
        Serial.printf("Connecting to MQTT with client id '%s'...", clientId.c_str());
        __backoff__ = __minbackoff__;
        while (true) {
            bool result = mqttClient->connect(clientId.c_str());

            if (mqttClient->lastError() != LWMQTT_SUCCESS && result) {
                Serial.printf("MQTT connection problem, error = %d (check lwmqtt_err_t), return code = %d (check lwmqtt_return_code_t)\n",
                    mqttClient->lastError(), mqttClient->returnCode());

                // See https://cloud.google.com/iot/docs/how-tos/exponential-backoff
                if (__backoff__ < __minbackoff__) {
                    __backoff__ = __minbackoff__;
                }
                __backoff__ = (__backoff__ * __factor__) + random(__jitter__);
                if (__backoff__ > __max_backoff__) {
                    __backoff__ = __max_backoff__;
                }

                // Clean up the client
                mqttClient->disconnect();
                Serial.println("Delaying " + String(__backoff__) + "ms");
                delay(__backoff__);
            } else {
                if (!mqttClient->connected()) {
                    Serial.println("Could not connect to MQTT, let's retry later...");
                    mqttClient->disconnect();
                    delay(__max_backoff__);
                } else {
                    // We're now connected
                    Serial.println("MQTT connected");
                    break;
                }
            }
        }

        // Set QoS to 1 (ack) for configuration messages
        this->mqttClient->subscribe(prefix + "/config", 1);
        // QoS 0 (no ack) for commands
        this->mqttClient->subscribe(prefix + "/commands/#", 0);
    }
}

bool MqttHandler::publishStatus(const JsonDocument& json) {
    return publish(prefix + "/status", json);
}

bool MqttHandler::publishTelemetry(const JsonDocument& json) {
    return publish(prefix + "/events", json);
}

bool MqttHandler::publish(const String& topic, const JsonDocument& json) {
    if (mqttClient == nullptr || !mqttClient->connected()) {
        return false;
    }
#ifdef DUMP_MQTT
    Serial.print("Publishing telemetry: ");
    serializeJsonPretty(json, Serial);
    Serial.println();
#endif
    String payload;
    serializeJson(json, payload);
    bool success = mqttClient->publish(topic, payload.c_str());
    if (!success) {
        Serial.printf("Error publishing to MQTT: %d\n", mqttClient->lastError());
    }
    return success;
}
