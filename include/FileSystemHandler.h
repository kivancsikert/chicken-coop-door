#pragma once

#if defined(ESP32)
#include <SPIFFS.h>
#elif defined(ESP8266)
#include <LittleFS.h>
#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"
#endif

class FileSystemHandler {
public:
    bool begin() {
#if defined(ESP32)
        if (!SPIFFS.begin()) {
            return false;
        }
#elif defined(ESP8266)
        LittleFSConfig cfg;
        cfg.setAutoFormat(true);
        LittleFS.setConfig(cfg);
        if (!LittleFS.begin()) {
            return false;
        }
#endif

        Serial.println("Contents:");
        File root = getFS().open("/", FILE_READ);
        while (true) {
            File file = root.openNextFile();
            if (!file) {
                break;
            }
            Serial.print(" - ");
            Serial.println(file.name());
        }
        return true;
    }

    FS& getFS() {
#if defined(ESP32)
        return SPIFFS;
#elif defined(ESP8266)
        return LittleFS;
#endif
    }
};
