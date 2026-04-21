#include "IRbabyUserSettings.h"
#include "IRbabySerial.h"
#include "IRbabyMQTT.h"
#include "WiFiManager.h"
#include "IRbabyGlobal.h"
#include "IRbabyIR.h"
#include "defines.h"
#include <LittleFS.h>
#ifdef ESP32
#include <WiFi.h>
#endif
StaticJsonDocument<1024> ConfigData;
StaticJsonDocument<1024> ACStatus;

static void settingsEnsureDefaults()
{
    ConfigData["version"] = FIRMWARE_VERSION;

    if (!ConfigData.containsKey("pin")) {
        ConfigData["pin"]["ir_send"] = T_IR;
        ConfigData["pin"]["ir_receive"] = R_IR;
    } else {
        if (!ConfigData["pin"].containsKey("ir_send"))
            ConfigData["pin"]["ir_send"] = T_IR;
        if (!ConfigData["pin"].containsKey("ir_receive"))
            ConfigData["pin"]["ir_receive"] = R_IR;
    }
}

bool settingsSave()
{
    DEBUGLN("Save Config");
    settingsEnsureDefaults();
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
#ifdef ESP8266
    FSInfo64 info;
    LittleFS.info64(info);
    DEBUGF("fs total bytes = %llu\n", info.totalBytes);
#elif defined(ESP32)
    FSInfo info;
    LittleFS.info(info);
    DEBUGF("fs total bytes = %u\n", info.totalBytes);
#endif
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
            settingsEnsureDefaults();
            serializeJsonPretty(ConfigData, Serial);
            Serial.println();
        }
        cache.close();
    } else {
        DEBUGLN("Don't exsit config");
    }
    settingsEnsureDefaults();

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
        cache.close();
    }
    ret = true;
    return ret;
}

void settingsClear()
{
#ifdef ESP8266
    ESP.eraseConfig();
#elif defined(ESP32)
    WiFi.disconnect(true, true);
#endif
    LittleFS.format();
    ESP.restart();
}

bool saveACStatus(String file, t_remote_ac_status status)
{
    ACStatus[file]["power"] = (int)status.ac_power;
    ACStatus[file]["temperature"] = (int)status.ac_temp;
    ACStatus[file]["mode"] = (int)status.ac_mode;
    ACStatus[file]["swing"] = (int)status.ac_swing;
    ACStatus[file]["speed"] = (int)status.ac_wind_speed;
    return true;
}

t_remote_ac_status getACState(String file)
{
    if (!ACStatus.containsKey(file))
        initAC(file);

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

void clearBinFiles() {
#ifdef ESP8266
    Dir root = LittleFS.openDir(BIN_SAVE_PATH);
    while (root.next()) {
        LittleFS.remove(BIN_SAVE_PATH + root.fileName());
    }
#elif defined(ESP32)
    File root = LittleFS.open(BIN_SAVE_PATH);
    if (root && root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
            String filePath = file.path();
            file.close();
            LittleFS.remove(filePath);
            file = root.openNextFile();
        }
    }
    root.close();
#endif
}
