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
    configTime(0, 0, "0.pool.ntp.org", "1.pool.ntp.org");
    Serial.println("Waiting on time sync...");
    while (time(nullptr) < 1510644967) {
        delay(10);
    }

    String projectId = config["projectId"];
    String location = config["location"];
    String registryId = config["registryId"];
    String deviceId = config["deviceId"];
    String privateKey = config["privateKey"];

    Serial.printf("Connecting to Google Cloud project '%s' in '%s', registry '%s' as device '%s'\n",
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
    mqtt->setUseLts(false);
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

    if (!mqttClient->connected()) {
        Serial.println("Reconnecting...");
        mqtt->mqttConnect();
    }
}

MqttReporter mqttReporter;

// To get the certificate for your region run:
//   openssl s_client -showcerts -connect mqtt.googleapis.com:8883
// for standard mqtt or for LTS:
//   openssl s_client -showcerts -connect mqtt.2030.ltsapis.goog:8883
// Copy the certificate (all lines between and including ---BEGIN CERTIFICATE---
// and --END CERTIFICATE--) to root.cert and put here on the root_cert variable.

const String root_cert = F("-----BEGIN CERTIFICATE-----\n"
                           "MIIESjCCAzKgAwIBAgINAeO0mqGNiqmBJWlQuDANBgkqhkiG9w0BAQsFADBMMSAw\n"
                           "HgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEGA1UEChMKR2xvYmFs\n"
                           "U2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjAeFw0xNzA2MTUwMDAwNDJaFw0yMTEy\n"
                           "MTUwMDAwNDJaMEIxCzAJBgNVBAYTAlVTMR4wHAYDVQQKExVHb29nbGUgVHJ1c3Qg\n"
                           "U2VydmljZXMxEzARBgNVBAMTCkdUUyBDQSAxTzEwggEiMA0GCSqGSIb3DQEBAQUA\n"
                           "A4IBDwAwggEKAoIBAQDQGM9F1IvN05zkQO9+tN1pIRvJzzyOTHW5DzEZhD2ePCnv\n"
                           "UA0Qk28FgICfKqC9EksC4T2fWBYk/jCfC3R3VZMdS/dN4ZKCEPZRrAzDsiKUDzRr\n"
                           "mBBJ5wudgzndIMYcLe/RGGFl5yODIKgjEv/SJH/UL+dEaltN11BmsK+eQmMF++Ac\n"
                           "xGNhr59qM/9il71I2dN8FGfcddwuaej4bXhp0LcQBbjxMcI7JP0aM3T4I+DsaxmK\n"
                           "FsbjzaTNC9uzpFlgOIg7rR25xoynUxv8vNmkq7zdPGHXkxWY7oG9j+JkRyBABk7X\n"
                           "rJfoucBZEqFJJSPk7XA0LKW0Y3z5oz2D0c1tJKwHAgMBAAGjggEzMIIBLzAOBgNV\n"
                           "HQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1Ud\n"
                           "EwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFJjR+G4Q68+b7GCfGJAboOt9Cf0rMB8G\n"
                           "A1UdIwQYMBaAFJviB1dnHB7AagbeWbSaLd/cGYYuMDUGCCsGAQUFBwEBBCkwJzAl\n"
                           "BggrBgEFBQcwAYYZaHR0cDovL29jc3AucGtpLmdvb2cvZ3NyMjAyBgNVHR8EKzAp\n"
                           "MCegJaAjhiFodHRwOi8vY3JsLnBraS5nb29nL2dzcjIvZ3NyMi5jcmwwPwYDVR0g\n"
                           "BDgwNjA0BgZngQwBAgIwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly9wa2kuZ29vZy9y\n"
                           "ZXBvc2l0b3J5LzANBgkqhkiG9w0BAQsFAAOCAQEAGoA+Nnn78y6pRjd9XlQWNa7H\n"
                           "TgiZ/r3RNGkmUmYHPQq6Scti9PEajvwRT2iWTHQr02fesqOqBY2ETUwgZQ+lltoN\n"
                           "FvhsO9tvBCOIazpswWC9aJ9xju4tWDQH8NVU6YZZ/XteDSGU9YzJqPjY8q3MDxrz\n"
                           "mqepBCf5o8mw/wJ4a2G6xzUr6Fb6T8McDO22PLRL6u3M4Tzs3A2M1j6bykJYi8wW\n"
                           "IRdAvKLWZu/axBVbzYmqmwkm5zLSDW5nIAJbELCQCZwMH56t2Dvqofxs6BBcCFIZ\n"
                           "USpxu6x6td0V7SvJCCosirSmIatj/9dSSVDQibet8q/7UK4v4ZUN80atnZz1yg==\n"
                           "-----END CERTIFICATE-----\n");
