#include "gsm.h"
#include "lilygo.h"

Gsm::Gsm(Config& config)
    : config(config)
#ifdef DUMP_AT_COMMANDS
    , debugger(StreamDebugger(SerialAT, Serial))
    , modem(TinyGsm(debugger))
#else
    , modem(TinyGsm(SerialAT))
#endif
    , client(TinyGsmClient(modem)) {
}

#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00

void Gsm::setupModem() {
#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Initialize the indicator as an output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LED_OFF);
}

void Gsm::enableNetLight(bool enable) {
    SerialMon.println("Turning SIM800 Red LED " + String(enable ? "on" : "off") + "...");
    modem.sendAT("+CNETLIGHT=" + String(enable ? "1" : "0"));
}

bool Gsm::begin(const String& rootCert) {
    if (config.gprsApn.isEmpty()) {
        Serial.println("No GPRS APN defined, not connecting");
        return false;
    }

    if (!config.gprsEnable) {
        Serial.println("GPRS not enabled");
        return false;
    }

    Serial.println("Initializing modem...");

    setupModem();

    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

    // TODO Add a command for this?
    // Restart SIM800 module, it takes quite some time
    // To skip it, call init() instead of restart()
    if (!modem.init()) {
        Serial.println("Could not initialize modem");
        return false;
    }
    // use modem.init() if you don't need the complete restart

    // Turn off network status lights to reduce current consumption
    // enableNetLight(false);

    Serial.println("Modem connected: " + modem.getModemInfo());

#ifdef DUMP_AT_COMMANDS
    // Enable detailed error reporting
    modem.sendAT("+CMEE=2");
    modem.waitResponse();
#endif

    // Unlock SIM card with a PIN if needed
    SimStatus simStatus = modem.getSimStatus();
    Serial.println("SIM status: " + String(simStatus));
    if (!config.simPin.isEmpty() && simStatus != 3) {
        Serial.println("Unlocking with PIN...");
        modem.simUnlock(config.simPin.c_str());
    }

    Serial.println("Waiting for network...");
    // TODO Make network wait configurable
    if (!modem.waitForNetwork(240000L) || !modem.isNetworkConnected()) {
        Serial.println("Could not connect to network");
        return false;
    }
    // TODO Report this as part of the device status
    Serial.println("Connected, GSM signal quality: " + String(modem.getSignalQuality()));

    // When the network connection is successful, turn on the indicator
    digitalWrite(LED_GPIO, LED_ON);

    Serial.println("Connecting to GPRS at " + config.gprsApn + "...");
    bool gprsConnectSuccessful = modem.gprsConnect(
        config.gprsApn.c_str(),
        config.gprsUsername.isEmpty() ? nullptr : config.gprsUsername.c_str(),
        config.gprsPassword.isEmpty() ? nullptr : config.gprsPassword.c_str());
    if (!gprsConnectSuccessful || !modem.isGprsConnected()) {
        Serial.println("Couldn't connect GPRS");
        return false;
    }

    Serial.println("GPRS connected");
    return true;
}
