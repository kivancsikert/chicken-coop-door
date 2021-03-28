#pragma once

#include <ArduinoJson.h>
#include <Client.h>
#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>
#include <MQTT.h>
#include <functional>

extern const String root_cert;

class MqttHandler {
public:
    MqttHandler();

    void begin(
        Client* netClient,
        const JsonDocument& config,
        std::function<void(JsonDocument&)> onConfigChange,
        std::function<void(JsonDocument&)> onCommand);
    void loop();

    void publishTelemetry(const JsonDocument& json);

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

    std::function<void(JsonDocument&)> onConfigChange;
    std::function<void(JsonDocument&)> onCommand;

    // Time (seconds) to expire token += 20 minutes for drift
    // Maximum 24H (3600*24)
    const int jwtExpirationInSeconds = 60 * 20;
};
