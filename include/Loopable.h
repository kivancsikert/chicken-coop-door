#pragma once

#include <Arduino.h>

class Loopable {
public:
    virtual void loop() = 0;
};

class TimedLoopable
    : public Loopable {
public:
    void loop() override {
        unsigned long currentTimeMillis = millis();
        if (currentTimeMillis - previousLoopMillis > getPeriodInMillis()) {
            previousLoopMillis = currentTimeMillis;
            timedLoop();
        }
    }

protected:
    virtual void timedLoop() = 0;
    virtual unsigned long getPeriodInMillis() = 0;

private:
    unsigned long previousLoopMillis = 0;
};
