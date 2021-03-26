#include "mqtt-reporter.h"
#include <SPIFFS.h>

MqttReporter::MqttReporter() {
}

String getJwt() {
    return mqttReporter.getJwt();
}

// The MQTT callback function for commands and configuration updates
// This is were incoming command from the gateway gets saved,
// to forward to the delegate device
void messageReceived(String& topic, String& payload) {
    mqttReporter.messageReceived(topic, payload);
}

void MqttReporter::begin(Client* netClient, const JsonDocument& config) {
    String projectId = config["projectId"];
    String location = config["location"];
    String registryId = config["registryId"];
    String deviceId = config["deviceId"];
    String privateKey = config["privateKey"];

    device = new CloudIoTCoreDevice(
        projectId.c_str(),
        location.c_str(),
        registryId.c_str(),
        deviceId.c_str(),
        privateKey.c_str());

    MQTTClient* mqttClient = new MQTTClient(360);
    mqttClient->setOptions(
        180,     // keepAlive
        true,    // cleanSession
        10000    // timeout
    );
    mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
    mqtt->setUseLts(true);
    mqtt->startMQTT();
    mqttClient->subscribe("/devices/" + deviceId + "/errors", 0);
}

String MqttReporter::getJwt() {
    time_t iss = time(nullptr);
    Serial.println("Refreshing JWT");
    String jwt = device->createJWT(iss, jwtExpirationInSeconds);
    Serial.println(jwt);
    return jwt;
}

void MqttReporter::messageReceived(String& topic, String& payload) {
    Serial.println("incoming: " + topic + " - " + payload);
}

void MqttReporter::loop() {
    mqtt->loop();
}

MqttReporter mqttReporter;
