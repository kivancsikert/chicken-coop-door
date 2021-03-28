#include "config.h"
#include <SPIFFS.h>
#include <limits>

template <typename T>
T getJsonValue(const JsonDocument& json, const String& key, T defaultValue) {
    if (json.containsKey(key)) {
        return json[key].as<T>();
    } else {
        return defaultValue;
    }
}

void Config::begin() {
    if (SPIFFS.exists("/config.json")) {
        File configFile = SPIFFS.open("/config.json", FILE_READ);
        DynamicJsonDocument configJson(configFile.size() * 2);
        deserializeJson(configJson, configFile);
        configFile.close();
        update(configJson);
        Serial.println("Loaded configuration");
    }
}

void Config::update(const JsonDocument& json) {
    openLightLimit = getJsonValue(json, "openLightLimit", std::numeric_limits<float>::min());
    closeLightLimit = getJsonValue(json, "openLightLimit", std::numeric_limits<float>::max());
    lightUpdateInterval = getJsonValue(json, "lightUpdateInterval", 1000);
    invertOpenSwitch = getJsonValue(json, "invertOpenSwitch", false);
    invertCloseSwitch = getJsonValue(json, "invertCloseSwitch", false);

    wifiSsid = getJsonValue(json, "wifiSsid", "");
    wifiPassword = getJsonValue(json, "wifiPassword", "");

    statePublishingInterval = getJsonValue(json, "statePublishingInterval", 5 * 1000);
}

void Config::store() {
    DynamicJsonDocument json(2048);
    json["openLightLimit"] = openLightLimit;
    json["closeLightLimit"] = closeLightLimit;
    json["lightUpdateInterval"] = lightUpdateInterval;
    json["invertOpenSwitch"] = invertOpenSwitch;
    json["invertCloseSwitch"] = invertCloseSwitch;

    json["wifiSsid"] = wifiSsid;
    json["wifiPassword"] = wifiPassword;

    json["statePublishingInterval"] = statePublishingInterval;

    File configFile = SPIFFS.open("/config.json", FILE_WRITE);
    serializeJson(json, configFile);
    configFile.close();
}
