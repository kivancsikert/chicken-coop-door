#include "MqttHandler.h"
#include <SPIFFS.h>

#include <SSLClient.h>
#ifdef USE_GOOGLE_LTS_DOMAIN
#include "google-iot-root-cert-ta-lts.h"
#else
#include "google-iot-root-cert-ta-non-lts.h"
#endif

MqttHandler* instance;

MqttHandler::MqttHandler() {
    instance = this;
}

String getJwt() {
    return instance->getJwt();
}

// The MQTT callback function for commands and configuration updates
void messageReceived(String& topic, String& payload) {
    instance->messageReceived(topic, payload);
}

void MqttHandler::begin(Client& netClient,
    const JsonDocument& config,
    std::function<void(JsonDocument&)> onConfigChange,
    std::function<void(JsonDocument&)> onCommand) {
    Serial.println("Initializing MQTT connector...");
    this->onConfigChange = onConfigChange;
    this->onCommand = onCommand;

    configTime(0, 0, "pool.ntp.org");
    while (true) {
        time_t currentTime = time(nullptr);
        if (currentTime > 1616800541) {
            Serial.printf("Current time is %ld\n", currentTime);
            break;
        }
        delay(10);
    }

    projectId = config["projectId"].as<String>();
    location = config["location"].as<String>();
    registryId = config["registryId"].as<String>();
    deviceId = config["deviceId"].as<String>();
    privateKey = config["privateKey"].as<String>();

    Serial.println("Connecting device '" + deviceId + "' to Google Cloud via MQTT on project '" + projectId + "' in '" + location + "' in registry '" + registryId + "'\n");
    device = new CloudIoTCoreDevice(projectId.c_str(), location.c_str(), registryId.c_str(), deviceId.c_str(), privateKey.c_str());

    mqttClient = new MQTTClient(MQTT_BUFFER_SIZE);
    mqttClient->setOptions(
        180,     // keepAlive
        true,    // cleanSession
        10000    // timeout
    );
    sslClient = new SSLClient(netClient, TAs, (size_t) TAs_NUM, A0);
    mqtt = new CloudIoTCoreMqtt(mqttClient, sslClient, device);
    mqtt->setLogConnect(false);
#ifdef USE_GOOGLE_LTS_DOMAIN
    mqtt->setUseLts(true);
#endif
    mqtt->startMQTT();
    mqtt->mqttConnect();
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
    mqtt->loop();

    if (!mqttClient->connected()) {
        Serial.println("Reconnecting...");
        mqtt->mqttConnect();
    }
}

void MqttHandler::publishState(const JsonDocument& json) {
    String payload;
    serializeJson(json, payload);
    mqtt->publishState(payload);
#ifdef DUMP_MQTT
    Serial.print("Published state: ");
    serializeJsonPretty(json, Serial);
    Serial.println();
#endif
}

void MqttHandler::publishTelemetry(const JsonDocument& json) {
    String payload;
    serializeJson(json, payload);
    mqtt->publishTelemetry(payload);
#ifdef DUMP_MQTT
    Serial.print("Published telemetry: ");
    serializeJsonPretty(json, Serial);
    Serial.println();
#endif
}
