#include "IRbabyUserSettings.h"
#include "IRbabySerial.h"
#include "IRbabyMQTT.h"
#include "WiFiManager.h"
#include "IRbabyGlobal.h"
#include "IRbabyIR.h"
#include "defines.h"
#include <LittleFS.h>
StaticJsonDocument<1024> ConfigData;
StaticJsonDocument<1024> ACStatus;

// void loadPin()
// {
// if (ConfigData.containsKey("send_pin")) {
//     int pin = ConfigData["send_pin"];
//     if (pin > 0) {
//         if (ir_send != NULL)
//             delete ir_send;
//         ir_send = new IRsend(pin);
//         ir_send->begin();
//         INFOF("Init gpio %d as IR send pin\n", pin);
//     }
// }
// if (ConfigData.containsKey("receive_pin")) {
//     int pin = ConfigData["receive_pin"];
//     if (pin > 0) {
//         if (ir_recv != NULL)
//             delete ir_recv;
//         const uint8_t kTimeout = 50;
//         const uint16_t kCaptureBufferSize = 1024;
//         ir_recv = new IRrecv(pin, kCaptureBufferSize, kTimeout, true);
// ir_recv->enableIRIn();
//         INFOF("Init gpio %d as IR receive pin\n", pin);
//     }
// }
// }

bool settingsSave()
{
    DEBUGLN("Save Config");
    serializeJsonPretty(ConfigData, Serial);
    File cache = LittleFS.open("/config", "w");
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
    // loadPin();
    mqttDisconnect();
    mqttReconnect();
    return true;
}

bool settingsLoad()
{
    LittleFS.begin();

    int ret = false;
    if (LittleFS.exists("/config"))
    {
        File cache = LittleFS.open("/config", "r");
        if (!cache)
        {
            ERRORLN("Failed to read config file");
            return ret;
        }
        if (cache.size() > 0)
        {
            DeserializationError error = deserializeJson(ConfigData, cache);
            if (error)
            {
                ERRORLN("Failed to load config settings");
                return ret;
            }
            INFOLN("Load config data:");
            ConfigData["version"] = FIRMWARE_VERSION;
            serializeJsonPretty(ConfigData, Serial);
            Serial.println();
        }
        cache.close();
    }

    if (LittleFS.exists("/acstatus")) {
        File cache = LittleFS.open("/acstatus", "r");
        if (!cache) {
            ERRORLN("Failed to read acstatus file");
            return ret;
        }
        if (cache.size() > 0) {
            DeserializationError error = deserializeJson(ACStatus, cache);
            if (error) {
                ERRORLN("Failed to load acstatus settings");
                return ret;
            }
        }
    }
    ret = true;
    return ret;
}

void settingsClear()
{
    DEBUGLN("\nReset settings");
    wifi_manager.resetSettings();
    LittleFS.format();
    ESP.reset();
}

bool saveACStatus(String file, t_remote_ac_status status)
{
    bool ret = false;
    ACStatus[file]["power"] = status.ac_power;
    ACStatus[file]["temperature"] = status.ac_temp;
    ACStatus[file]["mode"] = status.ac_mode;
    ACStatus[file]["swing"] = status.ac_swing;
    ACStatus[file]["speed"] = status.ac_wind_speed;
    File cache = LittleFS.open("/acstatus", "w");
    if (cache && (serializeJson(ACStatus, cache) == 0))
        ret = true;
    cache.close();
    return ret;
}