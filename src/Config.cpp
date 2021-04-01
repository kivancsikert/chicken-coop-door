#include "Config.h"
#include <SPIFFS.h>
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

void Config::begin() {
    if (SPIFFS.exists(CONFIG_FILE)) {
        File configFile = SPIFFS.open(CONFIG_FILE, FILE_READ);
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
    closeLightLimit = getJsonValue(json, "openLightLimit", std::numeric_limits<float>::max());
    lightUpdateInterval = getJsonValue(json, "lightUpdateInterval", 1000);
    lightLatencyInterval = getJsonValue(json, "lightLatencyInterval", 5000);

    invertOpenSwitch = getJsonValue(json, "invertOpenSwitch", false);
    invertCloseSwitch = getJsonValue(json, "invertCloseSwitch", false);

    wifiSsid = getJsonValue(json, "wifiSsid", "");
    wifiPassword = getJsonValue(json, "wifiPassword", "");

    simPin = getJsonValue(json, "simPin", "");
    gprsApn = getJsonValue(json, "gprsApn", "");
    gprsUsername = getJsonValue(json, "gprsUsername", "");
    gprsPassword = getJsonValue(json, "gprsPassword", "");
    gprsEnable = getJsonValue(json, "gprsEnable", true);

    statePublishingInterval = getJsonValue(json, "statePublishingInterval", 60 * 1000);
}

void Config::store() {
    DynamicJsonDocument json(2048);
    json["openLightLimit"] = openLightLimit;
    json["closeLightLimit"] = closeLightLimit;
    json["lightUpdateInterval"] = lightUpdateInterval;
    json["lightLatencyInterval"] = lightLatencyInterval;

    json["invertOpenSwitch"] = invertOpenSwitch;
    json["invertCloseSwitch"] = invertCloseSwitch;

    json["wifiSsid"] = wifiSsid;
    json["wifiPassword"] = wifiPassword;

    json["simPin"] = simPin;
    json["gprsApn"] = gprsApn;
    json["gprsUsername"] = gprsUsername;
    json["gprsPassword"] = gprsPassword;
    json["gprsEnable"] = gprsEnable;

    json["statePublishingInterval"] = statePublishingInterval;

    File configFile = SPIFFS.open(CONFIG_FILE, FILE_WRITE);
    serializeJson(json, configFile);
    configFile.close();
}
