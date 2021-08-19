#include "MqttHandler.h"

MqttHandler::MqttHandler(WiFiHandler& wifiHandler, NtpHandler& ntpHandler)
    : wifiHandler(wifiHandler)
    , ntpHandler(ntpHandler)
    , mqttClient(MQTT_BUFFER_SIZE) {
}

void MqttHandler::begin(
    const JsonDocument& mqttConfig,
    std::function<void(const JsonDocument&)> onConfigChange,
    std::function<void(const JsonDocument&)> onCommand) {

    Serial.println("Initializing MQTT connector...");

    String host = mqttConfig["host"].as<String>();
    int port = mqttConfig["port"].as<int>();
    clientId = mqttConfig["clientId"].as<String>();
    prefix = mqttConfig["prefix"].as<String>();

    Serial.printf("MQTT broker at '%s:%d', using client id '%s'\n",
        host.c_str(), port, clientId.c_str());

    mqttClient.setHost(host.c_str(), port);
    mqttClient.setKeepAlive(180);
    mqttClient.setCleanSession(true);
    mqttClient.setTimeout(10000);
    mqttClient.onMessage([onConfigChange, onCommand](String& topic, String& payload) {
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
    mqttClient.begin(wifiHandler.getClient());
}

void MqttHandler::loop() {
    if (!mqttClient.connected()) {
        if (!wifiHandler.connected()) {
            Serial.println("Couldn't connect to MQTT because WIFI is down");
            return;
        }

        if (!ntpHandler.isUpToDate()) {
            Serial.println("Couldn't connect to MQTT because NTP is not up to date");
            return;
        }

        __backoff__ = __minbackoff__;
        while (true) {
            Serial.print("Connecting to MQTT...");

            bool result = mqttClient.connect(clientId.c_str());

            if (mqttClient.lastError() != LWMQTT_SUCCESS && result) {
                Serial.printf("MQTT connection problem, error = %d (check lwmqtt_err_t), return code = %d (check lwmqtt_return_code_t)\n",
                    mqttClient.lastError(), mqttClient.returnCode());

                // See https://cloud.google.com/iot/docs/how-tos/exponential-backoff
                if (__backoff__ < __minbackoff__) {
                    __backoff__ = __minbackoff__;
                }
                __backoff__ = (__backoff__ * __factor__) + random(__jitter__);
                if (__backoff__ > __max_backoff__) {
                    __backoff__ = __max_backoff__;
                }

                // Clean up the client
                mqttClient.disconnect();
                Serial.println(" delaying " + String(__backoff__) + "ms");
                delay(__backoff__);
            } else {
                if (!mqttClient.connected()) {
                    Serial.println(" could not connect to MQTT, let's retry later...");
                    mqttClient.disconnect();
                    delay(__max_backoff__);
                } else {
                    // We're now connected
                    Serial.println(" connected");
                    break;
                }
            }
        }

        // Set QoS to 1 (ack) for configuration messages
        subscribe("config", 1);
        // QoS 0 (no ack) for commands
        subscribe("commands/#", 0);
    }

    mqttClient.loop();
}

bool MqttHandler::publishStatus(const JsonDocument& json) {
    return publish("status", json);
}

bool MqttHandler::publishTelemetry(const JsonDocument& json) {
    return publish("events", json);
}

bool MqttHandler::publish(const String& topic, const JsonDocument& json) {
    if (!mqttClient.connected()) {
        return false;
    }
    String fullTopic = prefix + "/" + topic;
#ifdef DUMP_MQTT
    Serial.printf("Publishing to MQTT topic '%s': ", fullTopic.c_str());
    serializeJsonPretty(json, Serial);
    Serial.println();
#endif
    String payload;
    serializeJson(json, payload);
    bool success = mqttClient.publish(fullTopic, payload.c_str());
    if (!success) {
        Serial.printf("Error publishing to MQTT topic at '%s', error = %d\n",
            fullTopic.c_str(), mqttClient.lastError());
    }
    return success;
}

bool MqttHandler::subscribe(const String& topic, int qos) {
    if (!mqttClient.connected()) {
        return false;
    }
    String fullTopic = prefix + "/" + topic;
    Serial.printf("Subscribing to MQTT topic '%s' with QOS = %d\n", fullTopic.c_str(), qos);
    bool success = mqttClient.subscribe(fullTopic.c_str(), qos);
    if (!success) {
        Serial.printf("Error subscribing to MQTT topic '%s', error = %d\n",
            fullTopic.c_str(), mqttClient.lastError());
    }
    return success;
}
