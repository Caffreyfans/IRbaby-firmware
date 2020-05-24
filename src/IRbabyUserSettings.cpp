#include <FS.h>
#include "IRbabyUserSettings.h"
#include "IRbabySerial.h"
#include "IRbabyMQTT.h"
#include "WiFiManager.h"

#define FIRMWARE_VERSION 0.3
StaticJsonDocument<1024> ConfigData;

IRsend* ir_send = NULL;
IRrecv* ir_recv = NULL;

void loadPin() {
    if (ConfigData.containsKey("send_pin")) {
        int pin = ConfigData["send_pin"];
        if (pin > 0) {
            if (ir_send != NULL)
                delete ir_send;
            ir_send = new IRsend(pin);
            ir_send->begin();
        }
    }
    if (ConfigData.containsKey("receive_pin")) {
        int pin = ConfigData["receive_pin"];
        if (pin > 0) {
            if (ir_recv != NULL)
                delete ir_recv;
            const uint8_t kTimeout = 50;
            const uint16_t kCaptureBufferSize = 1024;
            ir_recv = new IRrecv(pin, kCaptureBufferSize, kTimeout, true);
            ir_recv->enableIRIn();
            DEBUGF("load recv pin %d\n", pin);
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
            ConfigData["version"] = FIRMWARE_VERSION;
            serializeJsonPretty(ConfigData, IRBABY_DEBUG);
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
    WiFiManager wifi_manager;
    wifi_manager.resetSettings();
    SPIFFS.format();
    ESP.reset();
}