#pragma once

#if defined(ESP32)
#include <HTTPUpdate.h>
#elif defined(ESP8266)
#include <ESP8266httpUpdate.h>
#define httpUpdate ESPhttpUpdate
#endif

#include "WiFiHandler.h"

class HttpUpdateHandler {
public:
    HttpUpdateHandler(WiFiHandler& wifi)
        : wifi(wifi) {
        httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    }

    void update(const String& url, const String& version) {
        Serial.printf("Updating from version %s via URL %s\n", version.c_str(), url.c_str());
        WiFiClientSecure& client = wifi.getClient();
        HTTPUpdateResult result = httpUpdate.update(client, url, version);
        switch (result) {
            case HTTP_UPDATE_FAILED:
                Serial.println(httpUpdate.getLastErrorString());
                break;
            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("No updates available");
                break;
            case HTTP_UPDATE_OK:
                Serial.println("Update OK");
                break;
            default:
                Serial.println("Unknown response");
                break;
        }
    }

private:
    WiFiHandler& wifi;
};
