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
    File cache = LittleFS.open("/config", "w");
    if (!cache || (serializeJson(ConfigData, cache) == 0)) {
        ERRORLN("Failed to save config file");
        cache.close();
        return false;
    }
    cache.close();
    cache = LittleFS.open("/acstatus", "w");
    if (!cache || (serializeJson(ACStatus, cache) == 0))
    {
        ERRORLN("ERROR: Failed to save acstatus file");
        cache.close();
        return false;        
    }
    cache.close();
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
    } else {
        DEBUGLN("Don't exsit config");
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
            INFOLN("Load ACStatus data:");
            serializeJsonPretty(ACStatus, Serial);
            Serial.println();
        }
        cache.close();
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
    ACStatus[file]["power"] = (int)status.ac_power;
    ACStatus[file]["temperature"] = (int)status.ac_temp;
    ACStatus[file]["mode"] = (int)status.ac_mode;
    ACStatus[file]["swing"] = (int)status.ac_swing;
    ACStatus[file]["speed"] = (int)status.ac_wind_speed;
    return ret;
}

t_remote_ac_status getACState(String file)
{
    t_remote_ac_status status;
    int power = (int)ACStatus[file]["power"];
    int temperature = (int)ACStatus[file]["temperature"];
    int mode = (int)ACStatus[file]["mode"];
    int swing = (int)ACStatus[file]["swing"];
    int wind_speed = (int)ACStatus[file]["speed"];
    status.ac_power = (t_ac_power)power;
    status.ac_temp = (t_ac_temperature)temperature;
    status.ac_mode = (t_ac_mode)mode;
    status.ac_swing = (t_ac_swing)swing;
    status.ac_wind_speed = (t_ac_wind_speed)wind_speed;
    return status;
}