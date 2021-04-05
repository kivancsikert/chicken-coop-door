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

#include "Config.h"

class GprsHandler
    : ConfigAware {
public:
    GprsHandler(const Config& config)
        : ConfigAware(config)
#ifdef DUMP_AT_COMMANDS
        , debugger(StreamDebugger(SerialAT, Serial))
        , modem(TinyGsm(debugger))
#else
        , modem(TinyGsm(SerialAT))
#endif
        , client(TinyGsmClient(modem)) {
    }

    bool begin(const String& rootCert);

    TinyGsmClient& getClient() {
        return client;
    }

private:
    void setupModem();
    void enableNetLight(bool enable);

#ifdef DUMP_AT_COMMANDS
    StreamDebugger debugger;
#endif
    TinyGsm modem;
    TinyGsmClient client;
};
