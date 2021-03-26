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
    configTime(0, 0, "pool.ntp.org");
    while (true) {
        time_t currentTime = time(nullptr);
        if (currentTime > 1616800541) {
            Serial.printf("Current time is %ld\n", currentTime);
            break;
        }
        delay(10);
    }

    String projectId = config["projectId"];
    String location = config["location"];
    String registryId = config["registryId"];
    String deviceId = config["deviceId"];
    String privateKey = config["privateKey"];

    Serial.printf("Using Google Cloud via MQTT on project '%s' in '%s', registry '%s' as device '%s'\n",
        projectId.c_str(), location.c_str(), registryId.c_str(), deviceId.c_str());

    device = new CloudIoTCoreDevice(
        projectId.c_str(), location.c_str(), registryId.c_str(), deviceId.c_str(), privateKey.c_str());

    mqttClient = new MQTTClient(360);
    mqttClient->setOptions(
        180,     // keepAlive
        true,    // cleanSession
        10000    // timeout
    );
    mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
    mqtt->setUseLts(true);
    mqtt->startMQTT();

    // Subscribe to errors
    // TODO Is this a Google Cloud feature?
    mqttClient->subscribe("/devices/" + deviceId + "/errors", 0);

    // Subscribe to delegate configuration
    mqttClient->subscribe("/devices/" + deviceId + "/config", 1);

    // Subscribe to delegate commands
    mqttClient->subscribe("/devices/" + deviceId + "/commands/#", 0);

    mqtt->mqttConnect();

    delay(1500);

    Serial.printf("State topic: %s\n", device->getStateTopic().c_str());
    Serial.printf("Events topic: %s\n", device->getEventsTopic().c_str());
    Serial.printf("Publishing state: %d\n", mqtt->publishState("UP AND RUNNING"));
    // Serial.printf("Publishing telemetry: %d\n", mqtt->publishTelemetry("UP AND RUNNING"));
    // Serial.printf("Publishing: %d\n", mqttClient->publish("chicken-coop-door", "HELLO"));
    mqtt->loop();
}

String MqttReporter::getJwt() {
    time_t iss = time(nullptr);
    // Serial.println("Refreshing JWT");
    String jwt = device->createJWT(iss, jwtExpirationInSeconds);
    // Serial.println(jwt);
    return jwt;
}

void MqttReporter::messageReceived(String& topic, String& payload) {
    Serial.println("Received '" + topic + "': " + payload);
}

void MqttReporter::loop() {
    mqtt->loop();

    if (!mqttClient->connected()) {
        Serial.println("Reconnecting...");
        mqtt->mqttConnect();
    }
}

MqttReporter mqttReporter;

// To get the certificate for your region run:
//   openssl s_client -showcerts -connect mqtt.googleapis.com:8883
// for standard mqtt or for LTS, see
//   https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#downloading_mqtt_server_certificates
//   https://pki.goog/gtsltsr/gtsltsr.crt
// Copy the certificate (all lines between and including ---BEGIN CERTIFICATE---
// and --END CERTIFICATE--) to root.cert and put here on the root_cert variable.

const String root_cert = F("-----BEGIN CERTIFICATE-----\n"
                           "MIIBxTCCAWugAwIBAgINAfD3nVndblD3QnNxUDAKBggqhkjOPQQDAjBEMQswCQYD\n"
                           "VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzERMA8G\n"
                           "A1UEAxMIR1RTIExUU1IwHhcNMTgxMTAxMDAwMDQyWhcNNDIxMTAxMDAwMDQyWjBE\n"
                           "MQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExM\n"
                           "QzERMA8GA1UEAxMIR1RTIExUU1IwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATN\n"
                           "8YyO2u+yCQoZdwAkUNv5c3dokfULfrA6QJgFV2XMuENtQZIG5HUOS6jFn8f0ySlV\n"
                           "eORCxqFyjDJyRn86d+Iko0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUw\n"
                           "AwEB/zAdBgNVHQ4EFgQUPv7/zFLrvzQ+PfNA0OQlsV+4u1IwCgYIKoZIzj0EAwID\n"
                           "SAAwRQIhAPKuf/VtBHqGw3TUwUIq7TfaExp3bH7bjCBmVXJupT9FAiBr0SmCtsuk\n"
                           "miGgpajjf/gFigGM34F9021bCWs1MbL0SA==\n"
                           "-----END CERTIFICATE-----\n");
