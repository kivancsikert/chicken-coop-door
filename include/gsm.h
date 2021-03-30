#pragma once

#define SIM800C_AXP192_VERSION_20200609

// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

#define SerialMon Serial

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
#endif

#include "config.h"

class Gsm {
public:
    Gsm(Config& config);
    void begin(const String& rootCert);

    TinyGsmClient& getClient() {
        return client;
    }

private:
    void setupModem();
    void enableNetLight(bool enable);

    Config& config;

#ifdef DUMP_AT_COMMANDS
    StreamDebugger debugger;
#endif
    TinyGsm modem;
    TinyGsmClient client;
};
