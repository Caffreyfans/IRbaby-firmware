#include <FS.h>
#include "IRbabyUserSettings.h"
#include "IRbabySerial.h"
#include "IRbabyMQTT.h"

#define FIRMWARE_VERSION 0.1
StaticJsonDocument<1024> ConfigData;
IRsend* ir_send = new IRsend(0);

void loadPin();

void loadPin() {
    if (ConfigData.containsKey("send_pin")) {
        int pin = ConfigData["send_pin"];
        if (pin > 0) {
            delete ir_send;
            ir_send = new IRsend(pin);
            ir_send->begin();
        }
    }
}
bool settingsInit()
{
    return SPIFFS.begin();
}

bool settingsSave()
{
    DEBUGLN("Save Config");
    serializeJsonPretty(ConfigData, IRBABY_DEBUG);
    File cache = SPIFFS.open("/config", "w");
    if (!cache)
    {
        ERRORLN("ERROR: Failed to create file");
        return false;
    }

    if (serializeJson(ConfigData, cache) == 0)
    {
        ERRORLN("ERROR: Failed to write to file");
        return false;
    }
    cache.close();
    loadPin();
    mqttDisconnect();
    mqttReconnect();
    return true;
}

bool settingsLoad()
{
    if (SPIFFS.exists("/config"))
    {
        File cache = SPIFFS.open("/config", "r");
        if (!cache)
        {
            ERRORLN("Failed to read file");
            return false;
        }
        if (cache.size() > 0)
        {
            DeserializationError error = deserializeJson(ConfigData, cache);
            if (error)
            {
                ERRORLN("Failed to load settings");
                return false;
            }
            DEBUGLN("Load config data:");
            serializeJsonPretty(ConfigData, IRBABY_DEBUG);
            ConfigData["version"] = FIRMWARE_VERSION;
            DEBUGLN();
        }
        cache.close();
        loadPin();
    }
    return true;
}

void settingsClear()
{
    DEBUGLN("Reset settings");
    SPIFFS.format();
    ESP.reset();
}