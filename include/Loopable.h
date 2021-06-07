#pragma once

#include <Arduino.h>

template <class T>
class Loopable {
public:
    virtual T loop() = 0;
};

class TimedLoopable
    : public Loopable<void> {
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
