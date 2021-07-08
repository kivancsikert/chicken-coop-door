#pragma once

#include "Loopable.h"
#include "WiFiHandler.h"

class NtpHandler : public TimedLoopable<void> {
public:
    NtpHandler(WiFiHandler& wifiHandler)
        : wifiHandler(wifiHandler) {
    }

    void timedLoop() override {
        switch (state) {
            case State::UP_TO_DATE:
                if (millis() - lastChecked > 24 * 60 * 60 * 1000) {
                    state = State::NEEDS_UPDATE;
                }
                break;
            case State::NEEDS_UPDATE:
                if (!wifiHandler.connected()) {
                    Serial.println("Not updating NTP because WIFI is not available");
                    return;
                }
                Serial.println("Updating time from NTP");
                configTime(0, 0, "time.google.com");
                updateStarted = millis();
                state = State::UPDATING;
                break;
            case State::UPDATING:
                long currentTime = getTime();
                if (currentTime > 1616800541) {
                    Serial.printf("Current time is %ld\n", currentTime);
                    lastChecked = millis();
                    state = State::UP_TO_DATE;
                    break;
                }
                if (millis() - updateStarted > 10 * 1000) {
                    Serial.println("NPT update timed out");
                    state = State::NEEDS_UPDATE;
                }
                break;
        }
    }
    void defaultValue() override {
    }
    unsigned long getPeriodInMillis() {
        return 500;
    }

    long getTime() {
        return time(nullptr);
    }

    bool isUpToDate() {
        return state == State::UP_TO_DATE;
    }

private:
    unsigned long lastChecked = 0;

    enum class State {
        NEEDS_UPDATE,
        UPDATING,
        UP_TO_DATE
    };

    State state = State::NEEDS_UPDATE;
    unsigned long updateStarted;

    WiFiHandler& wifiHandler;
};
