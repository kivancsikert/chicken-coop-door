#include "gsm.h"
#include "lilygo.h"
#include "google-iot-root-cert.h"

const String rootCertFile = "ca.pem";

Gsm::Gsm(Config& config)
    : config(config)
#ifdef DUMP_AT_COMMANDS
    , debugger(StreamDebugger(SerialAT, Serial))
    , modem(TinyGsm(debugger))
#else
    , modem(TinyGsm(SerialAT))
#endif
    , client(TinyGsmClientSecure(modem)) {
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

void Gsm::begin(const String& rootCert) {
    if (config.gprsApn.isEmpty()) {
        Serial.println("No GPRS APN defined, not connecting");
        return;
    }

    Serial.println("Initializing modem...");

    setupModem();

    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

    // TODO Add a command for this?
    // Restart SIM800 module, it takes quite some time
    // To skip it, call init() instead of restart()
    modem.init();
    // use modem.init() if you don't need the complete restart

    // Turn off network status lights to reduce current consumption
    // enableNetLight(false);

    Serial.println("Modem connected: " + modem.getModemInfo());

#ifdef DUMP_AT_COMMANDS
    // Enable detailed error reporting
    modem.sendAT("+CMEE=2");
    modem.waitResponse();
#endif

    // Installing root CA
    modem.sendAT("+FSCREATE=", rootCertFile);
    if (modem.waitResponse() != 1) {
        // TODO Handle error
        return;
    }

    const int cert_size = rootCert.length();

    modem.sendAT("+FSWRITE=", rootCertFile, ",0,", cert_size, ",10");
    if (modem.waitResponse(">") != 1) {
        // TODO Handle error
        return;
    }

    for (int i = 0; i < cert_size; i++) {
        modem.stream.write(rootCert[i]);
    }

    modem.stream.write(GSM_NL);
    modem.stream.flush();

    if (modem.waitResponse(2000L) != 1) {
        // TODO Handle error
        return;
    }

    modem.sendAT("+SSLSETCERT=\"", rootCertFile, "\"");
    if (modem.waitResponse() != 1) {
        // TODO Handle error
        return;
    }
    if (modem.waitResponse(5000L, GSM_NL "+SSLSETCERT:") != 1) {
        // TODO Handle error
        return;
    }
    const int retCode = modem.stream.readStringUntil('\n').toInt();

    SerialMon.println();
    SerialMon.println();
    SerialMon.println(F("****************************"));
    SerialMon.print(F("Setting Certificate: "));
    SerialMon.println((0 == retCode) ? "OK" : "FAILED");
    SerialMon.println(F("****************************"));

    // Unlock SIM card with a PIN if needed
    SimStatus simStatus = modem.getSimStatus();
    Serial.println("SIM status: " + String(simStatus));
    if (!config.simPin.isEmpty() && simStatus != 3) {
        Serial.println("Unlocking with PIN...");
        modem.simUnlock(config.simPin.c_str());
    }

    Serial.print("Waiting for network...");
    // TODO Make network wait configurable
    if (!modem.waitForNetwork(240000L)) {
        Serial.println(" fail");
        // TODO Handle error
        delay(10000);
        return;
    }
    Serial.println(" success");
    // TODO Report this as part of the device status
    Serial.println("GSM signal quality: " + String(modem.getSignalQuality()));

    // When the network connection is successful, turn on the indicator
    digitalWrite(LED_GPIO, LED_ON);

    if (modem.isNetworkConnected()) {
        SerialMon.println("Network connected");
    }

    Serial.println("Connecting to GPRS at " + config.gprsApn + "...");
    modem.gprsConnect(
        config.gprsApn.c_str(),
        config.gprsUsername.isEmpty() ? nullptr : config.gprsUsername.c_str(),
        config.gprsPassword.isEmpty() ? nullptr : config.gprsPassword.c_str());
    if (modem.isGprsConnected()) {
        Serial.println("GPRS connected");
    } else {
        // TODO Handle error
        Serial.println("Couldn't connect GPRS");
    }

    // delay(1000);
    // Serial.println();
    // Serial.println();
    // Serial.println();

    // const char* server = "vsh.pp.ua";
    // const char* resource = "/TinyGSM/logo.txt";
    // Serial.print("Connecting to ");
    // Serial.println(server);
    // if (!client.connect(server, 443)) {
    //     Serial.println(" fail");
    //     delay(10000);
    //     return;
    // }
    // Serial.println(" success");

    // // Make a HTTP GET request:
    // // Server details
    // Serial.println("Performing HTTP GET request...");
    // client.print(String("GET ") + resource + " HTTP/1.1\r\n");
    // client.print(String("Host: ") + server + "\r\n");
    // client.print("Connection: close\r\n\r\n");
    // client.println();

    // uint32_t timeout = millis();
    // while (client.connected() && millis() - timeout < 10000L) {
    //     // Print available data
    //     while (client.available()) {
    //         char c = client.read();
    //         Serial.print(c);
    //         timeout = millis();
    //     }
    // }
    // Serial.println();
}
