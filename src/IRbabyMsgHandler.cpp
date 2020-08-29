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
#include "IRbabyRF.h"
#include "IRbabyGlobal.h"
#include "IRbabyha.h"

bool msgHandle(StaticJsonDocument<1024> *p_recv_msg_doc, MsgType msg_type)
{
    if (LOG_DEBUG)
    {
        serializeJsonPretty(*p_recv_msg_doc, Serial);
        Serial.println();
    }
    JsonObject obj = p_recv_msg_doc->as<JsonObject>();
    String cmd = obj["cmd"];
    JsonObject params = obj["params"];
    send_msg_doc.clear();

    if (cmd.equalsIgnoreCase("query"))
    {
        String type = params["type"];
        if (type.equals("discovery"))
        {
            String chip_id = String(ESP.getChipId(), HEX);
            chip_id.toUpperCase();
            send_msg_doc["cmd"] = "query_discovery";
            send_msg_doc["params"]["ip"] = WiFi.localIP().toString();
            send_msg_doc["params"]["mac"] = chip_id;

            if (ConfigData.containsKey("mqtt"))
            {
                send_msg_doc["params"]["mqtt"] = ConfigData["mqtt"];
            }

            if (ConfigData.containsKey("pin"))
            {
                send_msg_doc["params"]["pin"]= ConfigData["pin"];
            }

            if (ConfigData.containsKey("version"))
            {
                send_msg_doc["params"]["version"] = ConfigData["version"];
            }

            String ip = obj["params"]["ip"];
            remote_ip.fromString(ip);
            sendUDP(&send_msg_doc, remote_ip);
        }
        else if (type.equals("info"))
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
            send_msg_doc["cmd"] = "query_info";
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

    if (cmd.equalsIgnoreCase("send"))
    {
        String signal = params["signal"];
        String type = params["type"];

        if (signal.equalsIgnoreCase("ir"))
        {
            if (type.equalsIgnoreCase("status"))
            {
                String file = params["file"];
                JsonObject statusJson = params[type];
                t_remote_ac_status ac_status;
                ac_status.ac_power = t_ac_power((int)statusJson["power"]);
                ac_status.ac_temp = t_ac_temperature((int)statusJson["temperature"]);
                ac_status.ac_mode = t_ac_mode((int)statusJson["mode"]);
                ac_status.ac_swing = t_ac_swing((int)statusJson["swing"]);
                ac_status.ac_wind_speed = t_ac_wind_speed((int)statusJson["speed"]);
                ac_status.ac_display = 1;
                ac_status.ac_timer = 0;
                ac_status.ac_sleep = 0;
                sendStatus(file, ac_status);
                returnACStatus(file, ac_status);
            }
            else if (type.equalsIgnoreCase("file"))
            {
                String file = params["file"];
                sendIR(file);
            }
            else if (type.equalsIgnoreCase("key"))
            {
                String file = params["file"];
            }
            else if (type.equalsIgnoreCase("data"))
            {
            }
            else if (type.equalsIgnoreCase("local"))
            {
                String file = params["file"];
                JsonObject localobj = params[type];
                if (!ACStatus.containsKey(file)) {
                    initAC(file);
                }
                t_remote_ac_status ac_status = getACState(file);
                if (localobj.containsKey("mode")) {
                    String mode = localobj["mode"];
                    if (mode.equalsIgnoreCase("off"))
                        ac_status.ac_power = AC_POWER_OFF;
                    else
                        ac_status.ac_power = AC_POWER_ON;

                    if (mode.equalsIgnoreCase("cool"))
                        ac_status.ac_mode = AC_MODE_COOL;
                    else if (mode.equalsIgnoreCase("heat"))
                        ac_status.ac_mode = AC_MODE_HEAT;
                    else if (mode.equalsIgnoreCase("auto"))
                        ac_status.ac_mode = AC_MODE_AUTO;
                    else if (mode.equalsIgnoreCase("fan") || mode.equalsIgnoreCase("fan_only"))
                        ac_status.ac_mode = AC_MODE_FAN;
                    else if (mode.equalsIgnoreCase("dry"))
                        ac_status.ac_mode = AC_MODE_DRY;
                } else if (localobj.containsKey("temperature")) {
                    String temperature = localobj["temperature"];
                    ac_status.ac_temp = (t_ac_temperature)(temperature.toInt() - 16);
                } else if (localobj.containsKey("fan")) {
                    String fan = localobj["fan"];
                    if (fan.equalsIgnoreCase("auto"))
                        ac_status.ac_wind_speed = AC_WS_AUTO;
                    else if (fan.equalsIgnoreCase("low"))
                        ac_status.ac_wind_speed = AC_WS_LOW;
                    else if (fan.equalsIgnoreCase("medium"))
                        ac_status.ac_wind_speed = AC_WS_MEDIUM;
                    else if (fan.equalsIgnoreCase("high"))
                        ac_status.ac_wind_speed = AC_WS_HIGH;
                } else if (localobj.containsKey("swing")) {
                    String swing = localobj["swing"];
                    if (swing.equalsIgnoreCase("on"))
                        ac_status.ac_swing = AC_SWING_ON;
                    else if (swing.equalsIgnoreCase("off"))
                        ac_status.ac_swing = AC_SWING_OFF;
                }
                sendStatus(file, ac_status);
                returnACStatus(file, ac_status);
            }
        }
#ifdef USE_RF
        else if (signal.equalsIgnoreCase("rf315"))
        {
            RFTYPE rf_type;

            rf_type = RF315;
            if (type.equalsIgnoreCase("data"))
            {
                unsigned long code = params["code"];
                unsigned int length = params["length"];
                sendRFData(code, length, rf_type);
            }
            else if (type.equalsIgnoreCase("file"))
            {
                String file = params["file"];
                sendRFFile(file);
            }
        }
        else if (signal.equalsIgnoreCase("rf433"))
        {
            RFTYPE rf_type;
            if (type.equalsIgnoreCase("data"))
            {
                rf_type = RF433;
                unsigned long code = params["code"];
                unsigned int length = params["length"];
                sendRFData(code, length, rf_type);
            }
            else if (type.equalsIgnoreCase("file"))
            {
                String file = params["file"];
                sendRFFile(file);
            }
        }
#endif
    }

    if (cmd.equalsIgnoreCase("set"))
    {
        String type = params["type"];
        if (type.equals("update")) {
            String url = params["url"];
            otaUpdate(url);
        }

        else if (type.equals("record")) {
            String ip = params["ip"];
            remote_ip.fromString(ip);
            DEBUGLN("start record");
            enableIR();
            enableRF();
        }

        else if (type.equals("disable_record")) {
            DEBUGLN("disable record");
            disableRF();
            disableIR();
        }

        else if (type.equals("save_signal")) {
            String file_name = params["file"];
            String signal = params["signal"];
            if (signal.equals("IR"))
                saveIR(file_name);
            else
                saveRF(file_name);
        }

        else if (type.equals("reset"))
            settingsClear();

        else if (type.equals("config")) {
            if (params.containsKey("mqtt")) {
                ConfigData["mqtt"]["host"] = params["mqtt"]["host"];
                ConfigData["mqtt"]["port"] = params["mqtt"]["port"];
                ConfigData["mqtt"]["user"] = params["mqtt"]["user"];
                ConfigData["mqtt"]["password"] = params["mqtt"]["password"];
            }
            if (params.containsKey("pin")) {
                ConfigData["pin"]["ir_send"] = params["pin"]["ir_send"];
                ConfigData["pin"]["ir_receive"] = params["pin"]["ir_receive"];
            }
            send_msg_doc["cmd"] = "return";
            send_msg_doc["params"]["message"] = "set success";
            returnUDP(&send_msg_doc);
            settingsSave();
            mqttDisconnect();
            mqttReconnect();
            loadIRPin(ConfigData["pin"]["ir_send"], ConfigData["pin"]["ir_receive"]);
        }

        else if (type.equals("device")) {
            DEBUGLN("Register Device")
            String file = params["file"];
            int device_type = (int)params["device_type"];
            bool exist = params["exist"];
            if (device_type == 1)
                registAC(file, exist);
        }
    }
    return true;
}