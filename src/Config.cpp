#include "Config.h"

#include <limits>

#define CONFIG_FILE "/config.json"

template <typename T>
T getJsonValue(const JsonDocument& json, const String& key, T defaultValue) {
    if (json.containsKey(key)) {
        return json[key].as<T>();
    } else {
        return defaultValue;
    }
}

void Config::begin(bool reset) {
    if (reset) {
        Serial.println("Reset during startup, not reading configuration");
        return;
    }
    if (fileSystem.getFS().exists(CONFIG_FILE)) {
        File configFile = fileSystem.getFS().open(CONFIG_FILE, FILE_READ);
        DynamicJsonDocument configJson(configFile.size() * 2);
        deserializeJson(configJson, configFile);
        configFile.close();
        update(configJson);
        Serial.println("Effective configuration:");
        serializeJsonPretty(configJson, Serial);
        Serial.println();
    }
}

void Config::update(const JsonDocument& json) {
    openLightLimit = getJsonValue(json, "openLightLimit", std::numeric_limits<float>::min());
    closeLightLimit = getJsonValue(json, "closeLightLimit", std::numeric_limits<float>::max());
    lightUpdateInterval = getJsonValue(json, "lightUpdateInterval", 1000);
    lightLatencyInterval = getJsonValue(json, "lightLatencyInterval", 5000);

    motorEnabled = getJsonValue(json, "motorEnabled", true);
    movementTimeout = getJsonValue(json, "movementTimeout", 60 * 1000);
    invertOpenSwitch = getJsonValue(json, "invertOpenSwitch", false);
    invertCloseSwitch = getJsonValue(json, "invertCloseSwitch", false);

    wifiSsid = getJsonValue(json, "wifiSsid", "");
    wifiPassword = getJsonValue(json, "wifiPassword", "");

    statePublishingInterval = getJsonValue(json, "statePublishingInterval", 60 * 1000);
}

void Config::store() {
    DynamicJsonDocument json(2048);
    json["openLightLimit"] = openLightLimit;
    json["closeLightLimit"] = closeLightLimit;
    json["lightUpdateInterval"] = lightUpdateInterval;
    json["lightLatencyInterval"] = lightLatencyInterval;

    json["motorEnabled"] = motorEnabled;
    json["movementTimeout"] = movementTimeout;
    json["invertOpenSwitch"] = invertOpenSwitch;
    json["invertCloseSwitch"] = invertCloseSwitch;

    json["wifiSsid"] = wifiSsid;
    json["wifiPassword"] = wifiPassword;

    json["statePublishingInterval"] = statePublishingInterval;

    File configFile = fileSystem.getFS().open(CONFIG_FILE, FILE_WRITE);
    serializeJson(json, configFile);
    configFile.close();
}
