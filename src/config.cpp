#include "config.h"
#include <limits>

template <typename T>
T getJsonValue(const JsonDocument& json, const String& key, T defaultValue) {
    if (json.containsKey(key)) {
        return json[key].as<T>();
    } else {
        return defaultValue;
    }
}

void Config::load(const JsonDocument& json) {
    openLightLimit = getJsonValue(json, "openLightLimit", std::numeric_limits<float>::min());
    closeLightLimit = getJsonValue(json, "openLightLimit", std::numeric_limits<float>::max());
    invertOpenSwitch = getJsonValue(json, "invertOpenSwitch", false);
    invertCloseSwitch = getJsonValue(json, "invertCloseSwitch", false);

    wifiSsid = getJsonValue(json, "wifiSsid", "");
    wifiPassword = getJsonValue(json, "wifiPassword", "");
}

void Config::store(JsonDocument& json) {
    json["openLightLimit"] = openLightLimit;
    json["closeLightLimit"] = closeLightLimit;
    json["invertOpenSwitch"] = invertOpenSwitch;
    json["invertCloseSwitch"] = invertCloseSwitch;

    json["wifiSsid"] = wifiSsid;
    json["wifiPassword"] = wifiPassword;
}
