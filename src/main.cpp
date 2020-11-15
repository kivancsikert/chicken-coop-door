#include <Arduino.h>

#include <AccelStepper.h>
#include <BH1750.h>
#include <Wire.h>

#include <ESP8266WiFi.h>

#define OPEN_PIN D5
#define CLOSED_PIN D6

const int stepsPerRevolution = 2048;

AccelStepper motor(AccelStepper::FULL4WIRE, D0, D2, D1, D3);

BH1750 lightMeter;
float lightLimit = 70;

enum class State {
    OPEN,
    CLOSED,
    OPENING,
    CLOSING
};

State state = State::OPEN;

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

    WiFi.mode(WIFI_AP_STA);
    delay(500);
    WiFi.beginSmartConfig();
    Serial.print("WiFi connecting via SmartConfig");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        Serial.println(WiFi.smartConfigDone());
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
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
        if (lux < lightLimit && state == State::OPEN) {
            Serial.println("Closing...");
            state = State::CLOSING;
        } else if (lux >= lightLimit && state == State::CLOSED) {
            Serial.println("Opening...");
            state = State::OPENING;
        }
        Serial.printf("Light: %f, state: %d\n", lux, state);
    }

    if (!motor.run()) {
        if (state == State::OPEN || state == State::CLOSED) {
            delay(250);
            return;
        }
    }

    if (state == State::CLOSING) {
        int closed = digitalRead(CLOSED_PIN);
        if (closed) {
            Serial.println("Closed");
            motor.stop();
            state = State::CLOSED;
        } else {
            motor.move(stepsAtOnce);
        }
    } else if (state == State::OPENING) {
        int open = digitalRead(OPEN_PIN);
        if (open) {
            motor.stop();
            state = State::OPEN;
        } else {
            motor.move(-stepsAtOnce);
        }
    }
}
