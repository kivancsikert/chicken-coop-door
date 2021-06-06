#pragma once

#include "Loopable.h"
#include "WiFiHandler.h"

class NtpHandler : private Loopable<void> {
public:
    NtpHandler(WiFiHandler& wifiHandler)
        : wifiHandler(wifiHandler) {
    }

    void loop() override {
        if (millis() - lastChecked > 24 * 60 * 60 * 1000) {
            upToDate = false;

            if (!wifiHandler.getClient().connected()) {
                Serial.println("Not updating NTP because WIFI is not available");
                return;
            }

            Serial.println("Updating time from NTP");
            configTime(0, 0, "pool.ntp.org");
            unsigned long start = millis();
            while (millis() - start < 10 * 1000) {
                long currentTime = getTime();
                if (currentTime < 1616800541) {
                    delay(10);
                    continue;
                }
                Serial.printf("Current time is %ld\n", currentTime);
                lastChecked = millis();
                upToDate = true;
                break;
            }
        }
    }

    long getTime() {
        return time(nullptr);
    }

    bool isUpToDate() {
        return upToDate;
    }

private:
    unsigned long lastChecked = 0;
    bool upToDate = false;

    WiFiHandler& wifiHandler;
};
