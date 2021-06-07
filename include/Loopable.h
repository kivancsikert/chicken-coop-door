#pragma once

#include <Arduino.h>

template <class T>
class Loopable {
public:
    virtual T loop() = 0;
};

template <class T>
class TimedLoopable
    : public Loopable<T> {
public:
    T loop() override {
        unsigned long currentTimeMillis = millis();
        if (currentTimeMillis - previousLoopMillis > getPeriodInMillis()) {
            previousLoopMillis = currentTimeMillis;
            return timedLoop();
        }
        return defaultValue();
    }

protected:
    virtual T timedLoop() = 0;
    virtual T defaultValue() = 0;
    virtual unsigned long getPeriodInMillis() = 0;

private:
    unsigned long previousLoopMillis = 0;
};
