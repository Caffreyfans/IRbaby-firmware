#include "IRbabyMsgHandler.h"
#include "IRbabySerial.h"
#include "IRbabyUserSettings.h"
#include "IRbabyMQTT.h"
#include "IRbabyUDP.h"
#include "ESP8266WiFi.h"
#include "IRbabyIR.h"
#include "../lib/Irext/include/ir_ac_control.h"
#include "IRbabyOTA.h"
#include "defines.h"
#include <LittleFS.h>

void sendState(String file, t_remote_ac_status status);

bool msgHandle(StaticJsonDocument<1024> *p_recv_msg_doc, MsgType msg_type)
{
    if (LOG_DEBUG)
    {
        serializeJsonPretty(*p_recv_msg_doc, Serial);
        Serial.println();
    }
    JsonObject obj = p_recv_msg_doc->as<JsonObject>();
    String cmd = obj["cmd"];
    switch (msg_type)
    {
    case MsgType::mqtt:
    {
        String file = obj["params"]["file"];
        String var = obj["params"]["status"];
        /* 状态码处理 */
        if (cmd.length() > 0)
        {
            t_remote_ac_status ac_stauts = getACState(file);
            if (cmd.equals("mode"))
            {
                if (var.equals("off"))
                {
                    ac_stauts.ac_power = AC_POWER_OFF;
                }
                else
                {
                    ac_stauts.ac_power = AC_POWER_ON;
                }
                if (var.equals("cool"))
                    ac_stauts.ac_mode = (t_ac_mode)0;
                if (var.equals("heat"))
                    ac_stauts.ac_mode = (t_ac_mode)1;
                if (var.equals("auto"))
                    ac_stauts.ac_mode = (t_ac_mode)2;
                if (var.equals("fan"))
                    ac_stauts.ac_mode = (t_ac_mode)3;
                if (var.equals("dry"))
                    ac_stauts.ac_mode = (t_ac_mode)4;
            }
            if (cmd.equals("temperature"))
            {
                ac_stauts.ac_temp = (t_ac_temperature)(var.toInt() - 16);
            }
            if (cmd.equals("fan"))
            {
                if (var.equals("auto"))
                    ac_stauts.ac_wind_speed = (t_ac_wind_speed)0;
                if (var.equals("low"))
                    ac_stauts.ac_wind_speed = (t_ac_wind_speed)1;
                if (var.equals("medium"))
                    ac_stauts.ac_wind_speed = (t_ac_wind_speed)2;
                if (var.equals("high"))
                    ac_stauts.ac_wind_speed = (t_ac_wind_speed)3;
            }
            if (cmd.equals("swing"))
            {
                if (var.equals("on"))
                    ac_stauts.ac_swing = (t_ac_swing)0;
                if (var.equals("off"))
                    ac_stauts.ac_swing = (t_ac_swing)1;
            }
            if (cmd.equals("status"))
            {
                StaticJsonDocument<1024> var_doc;
                DeserializationError error = deserializeJson(var_doc, var);
                if (error)
                {
                    ERRORLN("Failed to parse var message");
                }
                ac_stauts.ac_power = t_ac_power((int)var_doc["power"]);
                ac_stauts.ac_temp = t_ac_temperature((int)var_doc["temperature"]);
                ac_stauts.ac_mode = t_ac_mode((int)var_doc["mode"]);
                ac_stauts.ac_swing = t_ac_swing((int)var_doc["swing"]);
                ac_stauts.ac_wind_speed = t_ac_wind_speed((int)var_doc["speed"]);
            }
            if (cmd.equals("raw"))
            {
                sendRaw(file);
            }
            else
            {
                sendStatus(file, ac_stauts);
            }
        }
    }
    break;

    case MsgType::udp:
    {
        send_msg_doc.clear();

        if (cmd.equals("config"))
        {
            JsonObject params = obj["params"];
            for (JsonPair kv : params)
            {
                ConfigData[kv.key()] = kv.value();
            }
            send_msg_doc["cmd"] = "return";
            send_msg_doc["params"]["message"] = "set success";
            returnUDP(&send_msg_doc);
            settingsSave();
        }

        if (cmd.equals("discovery"))
        {
            String chip_id = String(ESP.getChipId(), HEX);
            chip_id.toUpperCase();
            send_msg_doc["cmd"] = "upload";
            send_msg_doc["params"]["ip"] = WiFi.localIP().toString();
            send_msg_doc["params"]["mac"] = chip_id;

            if (ConfigData.containsKey("mqtt"))
            {
                send_msg_doc["params"]["mqtt"] = ConfigData["mqtt"];
            }

            if (ConfigData.containsKey("send_pin"))
            {
                send_msg_doc["params"]["send_pin"] = ConfigData["send_pin"];
            }

            if (ConfigData.containsKey("receive_pin"))
            {
                send_msg_doc["params"]["receive_pin"] = ConfigData["receive_pin"];
            }
            if (ConfigData.containsKey("version"))
            {
                send_msg_doc["params"]["version"] = ConfigData["version"];
            }

            String ip = obj["params"]["ip"];
            remote_ip.fromString(ip);
            sendUDP(&send_msg_doc, remote_ip);
        }

        if (cmd.equals("send"))
        {
            JsonObject params = obj["params"];
            String file_name = params["file"];
            if (params.containsKey("status"))
            {
                JsonObject statusJson = params["status"];
                t_remote_ac_status status;
                status.ac_power = t_ac_power((int)statusJson["power"]);
                status.ac_temp = t_ac_temperature((int)statusJson["temperature"]);
                status.ac_mode = t_ac_mode((int)statusJson["mode"]);
                status.ac_swing = t_ac_swing((int)statusJson["swing"]);
                status.ac_wind_speed = t_ac_wind_speed((int)statusJson["speed"]);
                status.ac_display = 1;
                status.ac_timer = 0;
                status.ac_sleep = 0;
                sendStatus(file_name, status);
            }
            else
            {
                sendRaw(file_name);
            }
        }

        if (cmd.equals("update"))
        {
            JsonObject params = obj["params"];
            String url = params["url"];
            otaUpdate(url);
        }

        if (cmd.equals("record"))
        {
            String ip = obj["params"];
            remote_ip.fromString(ip);
            ir_recv->enableIRIn();
        }

        if (cmd.equals("save"))
        {
            String file_name = obj["params"]["file"];
            saveRaw(file_name);
        }

        if (cmd.equals("reset"))
        {
            settingsClear();
        }

        if (cmd.equals("info"))
        {
            String free_mem = String(ESP.getFreeHeap() / 1024) + "KB";
            String chip_id = String(ESP.getChipId(), HEX);
            chip_id.toUpperCase();
            String cpu_freq = String(ESP.getCpuFreqMHz()) + "MHz";
            String flash_speed = String(ESP.getFlashChipSpeed() / 1000000);
            String flash_size = String(ESP.getFlashChipSize() / 1024) + "KB";
            String sketch_size = String(ESP.getSketchSize() / 1024) + "KB";
            String reset_reason = ESP.getResetReason();
            String sketch_space = String(ESP.getFreeSketchSpace() / 1024) + "KB";
            FSInfo fsinfo;
            LittleFS.info(fsinfo);
            String fs_total_bytes = String(fsinfo.totalBytes / 1024) + "KB";
            String fs_used_bytes = String(fsinfo.usedBytes / 1024) + "KB";
            send_msg_doc["cmd"] = "info_rt";
            send_msg_doc["params"]["free_mem"] = free_mem;
            send_msg_doc["params"]["chip_id"] = chip_id;
            send_msg_doc["params"]["cpu_freq"] = cpu_freq;
            send_msg_doc["params"]["flash_speed"] = flash_speed;
            send_msg_doc["params"]["flash_size"] = flash_size;
            send_msg_doc["params"]["reset_reason"] = reset_reason;
            send_msg_doc["params"]["sketch_space"] = sketch_space;
            send_msg_doc["params"]["fs_total_bytes"] = fs_total_bytes;
            send_msg_doc["params"]["fs_used_bytes"] = fs_used_bytes;
            send_msg_doc["params"]["version_name"] = FIRMWARE_VERSION;
            send_msg_doc["params"]["version_code"] = VERSION_CODE;
            returnUDP(&send_msg_doc);
        }
    }
    break;

    default:
        break;
    }
    return true;
}

void sendState(String file_name, t_remote_ac_status status)
{
    String chip_id = String(ESP.getChipId(), HEX);
    String topic_head = "/IRbaby/" + chip_id + "/state/" + file_name + "/";
    String topic = topic_head + "mode";
    String payload;
    const char *mode[5] = {"auto", "cool", "test", "a", "b"};
    int index = (int)status.ac_mode;
    DEBUGLN("mode is");
    DEBUGLN(mode[index]);
}