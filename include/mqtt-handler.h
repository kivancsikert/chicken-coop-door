#pragma once

#include <ArduinoJson.h>
#include <Client.h>
#include <MQTT.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>

#include "config.h"

extern const String root_cert;

class MqttHandler {
public:
    MqttHandler(Config& config);

    void begin(Client* netClient, const JsonDocument& config);
    void loop();

private:
    String getJwt();
    friend String getJwt();

    void messageReceived(String& topic, String& payload);
    friend void messageReceived(String& topic, String& payload);

    String projectId;
    String location;
    String registryId;
    String deviceId;
    String privateKey;

    CloudIoTCoreMqtt* mqtt;
    MQTTClient* mqttClient;
    CloudIoTCoreDevice* device;

    Config& config;

    // Time (seconds) to expire token += 20 minutes for drift
    // Maximum 24H (3600*24)
    const int jwtExpirationInSeconds = 60 * 20;
};
