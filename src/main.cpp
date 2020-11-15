#include <Arduino.h>

#include <AccelStepper.h>
#include <BH1750.h>
#include <Wire.h>

#define OPEN_PIN D5
#define CLOSED_PIN D6

const int stepsPerRevolution = 2048;

AccelStepper motor(AccelStepper::FULL4WIRE, D0, D2, D1, D3);

BH1750 lightMeter;
float lightLimit = 70;

enum State {
    OPEN,
    CLOSED,
    OPENING,
    CLOSING
};

State state = OPEN;

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(OPEN_PIN, INPUT);
    pinMode(CLOSED_PIN, INPUT);

    while (!Serial) {
        delay(100);
    }

    motor.setMaxSpeed(500);
    motor.setSpeed(500);
    motor.setAcceleration(500);

    Wire.begin(9, 10);
    if (lightMeter.begin()) {
        Serial.println("BH1750 initialised");
    } else {
        Serial.println("Error initialising BH1750");
    }
}

unsigned long previousMillis = 0;
unsigned long interval = 1000;

unsigned int stepsAtOnce = 100;

void loop()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        float lux = lightMeter.readLightLevel();
        if (lux < lightLimit && state == OPEN) {
            Serial.println("Closing...");
            state = CLOSING;
        } else if (lux >= lightLimit && state == CLOSED) {
            Serial.println("Opening...");
            state = OPENING;
        }
        Serial.printf("Light: %f, state: %d\n", lux, state);
    }

    if (!motor.run()) {
        if (state == OPEN || state == CLOSED) {
            delay(250);
            return;
        }
    }

    if (state == CLOSING) {
        int closed = digitalRead(CLOSED_PIN);
        if (closed) {
            Serial.println("Closed");
            motor.stop();
            state = CLOSED;
        } else {
            motor.move(stepsAtOnce);
        }
    } else if (state == OPENING) {
        int open = digitalRead(OPEN_PIN);
        if (open) {
            motor.stop();
            state = OPEN;
        } else {
            motor.move(-stepsAtOnce);
        }
    }
}
