#pragma once

#include <ArduinoJson.h>
#include <Client.h>
#include <MQTT.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>

class MqttReporter {
public:
    MqttReporter();

    void begin(Client* netClient, const JsonDocument& config);
    void loop();

private:
    String getJwt();
    friend String getJwt();

    void messageReceived(String& topic, String& payload);
    friend void messageReceived(String& topic, String& payload);

    CloudIoTCoreMqtt* mqtt;
    CloudIoTCoreDevice* device;
    int jwtExpirationInSeconds = 3600;
};

extern MqttReporter mqttReporter;
