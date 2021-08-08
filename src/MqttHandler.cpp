#include "MqttHandler.h"

MqttHandler* instance;

MqttHandler::MqttHandler(WiFiHandler& wifiHandler, NtpHandler& ntpHandler)
    : wifiHandler(wifiHandler)
    , ntpHandler(ntpHandler) {
    instance = this;
}

String getJwt() {
    return instance->getJwt();
}

// The MQTT callback function for commands and configuration updates
void messageReceived(String& topic, String& payload) {
    instance->messageReceived(topic, payload);
}

void MqttHandler::begin(
    const JsonDocument& config,
    std::function<void(JsonDocument&)> onConfigChange,
    std::function<void(JsonDocument&)> onCommand) {
    Serial.println("Initializing MQTT connector...");
    this->onConfigChange = onConfigChange;
    this->onCommand = onCommand;

    projectId = config["projectId"].as<String>();
    location = config["location"].as<String>();
    registryId = config["registryId"].as<String>();
    deviceId = config["deviceId"].as<String>();
    privateKey = config["privateKey"].as<String>();
}

String MqttHandler::getJwt() {
    time_t iss = time(nullptr);
    // Serial.println("Refreshing JWT");
    String jwt = device->createJWT(iss, JWT_EXPIRATION_IN_SECONDS);
    // Serial.println(jwt);
    return jwt;
}

void MqttHandler::messageReceived(const String& topic, const String& payload) {
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
}

void MqttHandler::loop() {
    if (mqtt == nullptr) {
        if (!wifiHandler.connected()) {
            Serial.println("Couldn't connect to MQTT because WIFI is down");
            return;
        }

        if (!ntpHandler.isUpToDate()) {
            Serial.println("Couldn't connect to MQTT because NTP is not up to date");
            return;
        }

        Serial.println("Using Google Cloud device '" + deviceId + "' on project '" + projectId + "' in '" + location + "' in registry '" + registryId + "'\n");
        device = new CloudIoTCoreDevice(projectId.c_str(), location.c_str(), registryId.c_str(), deviceId.c_str(), privateKey.c_str());

        mqttClient = new MQTTClient(MQTT_BUFFER_SIZE);
        mqttClient->setOptions(
            180,     // keepAlive
            true,    // cleanSession
            10000    // timeout
        );
        mqtt = new CloudIoTCoreMqtt(mqttClient, &wifiHandler.getClient(), device);
        mqtt->setLogConnect(false);
#ifdef USE_GOOGLE_LTS_DOMAIN
        mqtt->setUseLts(true);
#endif
        mqtt->startMQTT();
    }

    if (!mqttClient->connected()) {
        Serial.println("Connecting to MQTT...");
        mqtt->mqttConnectAsync();
    }

    mqtt->loop();
}

bool MqttHandler::publishStatus(const JsonDocument& json) {
    if (mqttClient == nullptr || !mqttClient->connected()) {
        return false;
    }
    String payload;
    serializeJson(json, payload);
    bool success = mqtt->publishTelemetry("/status", payload);
#ifdef DUMP_MQTT
    Serial.print("Published status: ");
    serializeJsonPretty(json, Serial);
    Serial.println();
#endif
    return success;
}

bool MqttHandler::publishTelemetry(const JsonDocument& json) {
    if (mqttClient == nullptr || !mqttClient->connected()) {
        return false;
    }
    String payload;
    serializeJson(json, payload);
    bool success = mqtt->publishTelemetry(payload);
#ifdef DUMP_MQTT
    Serial.print("Published telemetry: ");
    serializeJsonPretty(json, Serial);
    Serial.println();
#endif
    return success;
}
