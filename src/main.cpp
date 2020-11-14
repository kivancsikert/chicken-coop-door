#include <Arduino.h>

#include <BH1750.h>
#include <AccelStepper.h>
#include <Wire.h>

#define RIGHT_PIN D5
#define LEFT_PIN D6

const int stepsPerRevolution = 2048;

AccelStepper motor(AccelStepper::FULL4WIRE, D0, D2, D1, D3);

int speed = 10;

BH1750 lightMeter;

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RIGHT_PIN, INPUT);
    pinMode(LEFT_PIN, INPUT);

    while (!Serial) {
        delay(100);
    }

    motor.setMaxSpeed(500);
    motor.setAcceleration(200);

    // Wire.begin(D1, D2);
    // if (lightMeter.begin()) {
    //     Serial.println("BH1750 initialised");
    // } else {
    //     Serial.println("Error initialising BH1750");
    // }
    // Serial.println("Running...");
}

void loop()
{
    int right = digitalRead(RIGHT_PIN);
    int left = digitalRead(LEFT_PIN);
    if (left != 0 || right != 0) {
        int target = 2038 * 2 * (left - right);
        Serial.printf("Moving to %d\n", target);
        motor.moveTo(target);
    }
    motor.run();

    // uint16_t lux = lightMeter.readLightLevel();
    // Serial.print("Light: ");
    // Serial.print(lux);
    // Serial.println(" lx");
    // delay(1000);
}
