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
    DynamicJsonDocument configJson(2048);
    if (reset) {
        Serial.println("Reset during startup, falling back to default configuration");
    } else if (!fileSystem.getFS().exists(CONFIG_FILE)) {
        Serial.println("Configuration not found, falling back to default configuration");
    } else {
        Serial.println("Found config file, loading");
        File configFile = fileSystem.getFS().open(CONFIG_FILE, FILE_READ);
        deserializeJson(configJson, configFile);
        configFile.close();
    }
    update(configJson);

    // Print effective configuration
    serialize(configJson);
    Serial.println("Effective configuration:");
    serializeJsonPretty(configJson, Serial);
    Serial.println();
}

void Config::update(const JsonDocument& json) {
    openLightLimit = getJsonValue(json, "openLightLimit", std::numeric_limits<float>::max());
    closeLightLimit = getJsonValue(json, "closeLightLimit", std::numeric_limits<float>::min());
    lightUpdateInterval = getJsonValue(json, "lightUpdateInterval", 1000);
    lightLatencyInterval = getJsonValue(json, "lightLatencyInterval", 5000);

    motorEnabled = getJsonValue(json, "motorEnabled", true);
    movementTimeout = getJsonValue(json, "movementTimeout", 60 * 1000);
    invertOpenSwitch = getJsonValue(json, "invertOpenSwitch", true);
    invertCloseSwitch = getJsonValue(json, "invertCloseSwitch", true);

    wifiSsid = getJsonValue(json, "wifiSsid", "");
    wifiPassword = getJsonValue(json, "wifiPassword", "");
    wifiConnectionTimeout = getJsonValue(json, "wifiConnectionTimeout", 20 * 1000);

    statePublishingInterval = getJsonValue(json, "statePublishingInterval", 60 * 1000);
}

void Config::store() {
    DynamicJsonDocument json(2048);
    serialize(json);
    File configFile = fileSystem.getFS().open(CONFIG_FILE, FILE_WRITE);
    serializeJson(json, configFile);
    configFile.close();
}

void Config::serialize(JsonDocument& json) {
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
    json["wifiConnectionTimeout"] = wifiConnectionTimeout;

    json["statePublishingInterval"] = statePublishingInterval;
}
