#include "WiFiHandler.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

void WiFiHandler::begin(const String& hostname) {
    bool wifiModeSuccessful = WiFi.mode(WIFI_AP_STA);
    if (!wifiModeSuccessful) {
        Serial.println("WIFI mode unsuccessful");
    }
    delay(500);
    int retries = 3;
    while (true) {
        startWifi();
        if (awaitConnect()) {
            break;
        }
        WiFi.stopSmartConfig();
        if (--retries > 0) {
            continue;
        }
        Serial.println("Failed to connect WIFI, restarting");
        ESP.restart();
    }

    Serial.print(" connected, IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print(", hostname: ");
#if defined(ESP32)
    WiFi.setHostname(hostname.c_str());
    Serial.println(WiFi.getHostname());

    client.setCACert(caCert.c_str());
#elif defined(ESP8266)
    WiFi.hostname(hostname);
    Serial.println(WiFi.hostname());

    // TODO Replace this with a non-deprecated variant
    // client.setCACert((const uint8_t*) caCert.c_str(), caCert.length() + 1);
    client.setTrustAnchors(caCert);
#endif
}

void WiFiHandler::startWifi() {
    if (!config.wifiSsid.isEmpty()) {
        Serial.print("Using stored WIFI configuration to connect to ");
        Serial.print(config.wifiSsid);
        Serial.print("...");
        WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
    } else {
        Serial.print("WIFI is not configured, using SmartConfig...");
        bool smartConfigBeginSuccess = WiFi.beginSmartConfig();
        if (!smartConfigBeginSuccess) {
            Serial.print(" unsuccessful");
        }
    }
}

bool WiFiHandler::awaitConnect() {
    unsigned long startTime = millis();
    while (true) {
        wl_status_t status = WiFi.status();
        switch (status) {
            case WL_CONNECTED:
                return true;
            case WL_CONNECT_FAILED:
                Serial.println("WIFI connection failed");
                return false;
            default:
                break;
        }
        // TODO Make WIFI connection timeout configurable
        if (millis() - startTime > 10 * 1000) {
            Serial.println("WIFI connection timed out");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
}
