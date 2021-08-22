#include "MqttHandler.h"

#include <ESPmDNS.h>

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

    host = mqttConfig["host"].as<String>();
    port = mqttConfig["port"].as<int>();
    clientId = mqttConfig["clientId"].as<String>();
    prefix = mqttConfig["prefix"].as<String>();

    Serial.printf("MQTT broker at '%s:%d', using client id '%s'\n",
        host.c_str(), port, clientId.c_str());

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
        } else if (topic.endsWith("/command")) {
            onCommand(json);
        } else {
            Serial.printf("Unknown topic: '%s'\n", topic.c_str());
        }
    });
    mqttClient.begin(wifiHandler.getClient());
}

void MqttHandler::timedLoop() {
    if (!mqttClient.connected()) {
        if (!tryConnect()) {
            return;
        }
    }
    mqttClient.loop();
}

bool MqttHandler::tryConnect() {
    if (!wifiHandler.connected()) {
        Serial.println("Couldn't connect to MQTT because WIFI is down");
        return false;
    }

    if (!ntpHandler.isUpToDate()) {
        Serial.println("Couldn't connect to MQTT because NTP is not up to date");
        return false;
    }

    Serial.printf("Connecting to MQTT at %s", host.c_str());

    // Lookup host name via MDNS explicitly
    // See https://github.com/kivancsikert/chicken-coop-door/issues/128
    String mdnsHost = host;
    if (mdnsHost.endsWith(".local")) {
        mdnsHost = mdnsHost.substring(0, mdnsHost.length() - 6);
    }
    IPAddress address = MDNS.queryHost(mdnsHost);
    if (address == IPAddress()) {
        Serial.print(" using the host name");
        mqttClient.setHost(host.c_str(), port);
    } else {
        Serial.print(" using IP " + address.toString());
        mqttClient.setHost(address, port);
    }
    Serial.print("...");

    bool result = mqttClient.connect(clientId.c_str());

    if (!result) {
        Serial.printf(" failed, error = %d (check lwmqtt_err_t), return code = %d (check lwmqtt_return_code_t)\n",
            mqttClient.lastError(), mqttClient.returnCode());

        // Clean up the client
        mqttClient.disconnect();
        return false;
    }

    // We're now connected
    Serial.println(" connected");

    // Set QoS to 1 (ack) for configuration messages
    subscribe("config", 1);
    // QoS 0 (no ack) for commands
    subscribe("command", 0);
    return true;
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
