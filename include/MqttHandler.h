#pragma once

#include <ArduinoJson.h>
#include <Client.h>
#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>
#include <MQTT.h>
#include <functional>

#include "Loopable.h"
#include "NtpHandler.h"
#include "WiFiHandler.h"

#define MQTT_BUFFER_SIZE 2048

// Time (seconds) to expire token += 20 minutes for drift
// Maximum 24H (3600 * 24)
#define JWT_EXPIRATION_IN_SECONDS (60 * 60)

class MqttHandler : Loopable<void> {
public:
    MqttHandler(WiFiHandler& wifiHandler, NtpHandler& ntpHandler);

    void begin(
        const JsonDocument& config,
        std::function<void(JsonDocument&)> onConfigChange,
        std::function<void(JsonDocument&)> onCommand);
    void loop() override;

    bool publishStatus(const JsonDocument& json);
    bool publishTelemetry(const JsonDocument& json);

private:
    String getJwt();
    friend String getJwt();

    void messageReceived(const String& topic, const String& payload);
    friend void messageReceived(String& topic, String& payload);

    String projectId;
    String location;
    String registryId;
    String deviceId;
    String privateKey;

    MQTTClient* mqttClient;
    CloudIoTCoreMqtt* mqtt;
    CloudIoTCoreDevice* device;

    std::function<void(JsonDocument&)> onConfigChange;
    std::function<void(JsonDocument&)> onCommand;

    WiFiHandler& wifiHandler;
    NtpHandler& ntpHandler;
};
