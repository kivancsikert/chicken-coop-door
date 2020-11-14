#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter;

void setup()
{
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    Wire.begin(D1, D2);
    if (lightMeter.begin()) {
        Serial.println("BH1750 initialised");
    } else {
        Serial.println("Error initialising BH1750");
    }
    Serial.println("Running...");
}

void loop()
{
    uint16_t lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    delay(1000);
}
