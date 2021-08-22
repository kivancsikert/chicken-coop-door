#include "WiFiHandler.h"

#if defined(ESP32)
#include <ESPmDNS.h>
#endif

void WiFiHandler::begin() {
    bool wifiModeSuccessful = WiFi.mode(WIFI_STA);
    if (!wifiModeSuccessful) {
        Serial.println("WIFI mode unsuccessful");
    }
}

bool WiFiHandler::timedLoop() {
    if (connecting) {
        awaitConnect();
        return false;
    } else if (connected()) {
        return true;
    } else {
        startConnecting();
        return false;
    }
}

void WiFiHandler::startConnecting() {
    if (!config.wifiSsid.isEmpty()) {
        Serial.print("Using stored WIFI configuration to connect to ");
        Serial.print(config.wifiSsid);
        Serial.print("...");
        WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
    } else {
        Serial.print("WIFI is not configured, using SmartConfig...");
        bool smartConfigBeginSuccess = WiFi.beginSmartConfig();
        if (!smartConfigBeginSuccess) {
            Serial.println(" couldn't init SmartConfig, giving up");
            stopConnecting();
            return;
        }
    }
    connectionStarted = millis();
    connecting = true;
}

bool WiFiHandler::awaitConnect() {
    wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED) {
        connecting = false;
        Serial.print(" connected, IP address: ");
        Serial.print(WiFi.localIP());
        Serial.print(", hostname: ");
#if defined(ESP32)
        if (!MDNS.begin(hostname.c_str())) {
            Serial.println("MDNS.begin() failed");
        }
        WiFi.setHostname(hostname.c_str());
        Serial.println(WiFi.getHostname());
#elif defined(ESP8266)
        WiFi.hostname(hostname);
        Serial.println(WiFi.hostname());
#endif
        // To allow accessing HTTPS content for updates
        secureClient.setInsecure();
        return true;
    }

    Serial.print(".");
    if (status == WL_CONNECT_FAILED) {
        // There doesn't seem to be a better way to work around this
        // See https://github.com/esp8266/Arduino/issues/5527
        Serial.println("WIFI connection failed, restarting");
        delay(1000);
        ESP.restart();
    } else if (millis() - connectionStarted > config.wifiConnectionTimeout) {
        Serial.println("WIFI connection timed out");
        stopConnecting();
    }
    return false;
}

void WiFiHandler::stopConnecting() {
    connecting = false;
    WiFi.stopSmartConfig();
    WiFi.disconnect();
}
