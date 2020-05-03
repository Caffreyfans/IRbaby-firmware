#include "IRbabyMsgHandler.h"
#include "IRbabySerial.h"
#include "IRbabyUserSettings.h"
#include "IRbabyMQTT.h"
#include "IRbabyUDP.h"
#include "ESP8266WiFi.h"
#include "IRbabyIR.h"
#include "../lib/Irext/include/ir_ac_control.h"
#include "IRbabyOTA.h"

StaticJsonDocument<1024> send_msg_doc;
StaticJsonDocument<1024> recv_msg_doc;

void sendState(String file, t_remote_ac_status status);

bool msgHandle(StaticJsonDocument<1024> *p_recv_msg_doc, MsgType msg_type)
{
    serializeJsonPretty(*p_recv_msg_doc, IRBABY_DEBUG);
    IRBABY_DEBUG.println();
    JsonObject obj = p_recv_msg_doc->as<JsonObject>();
    switch (msg_type)
    {
        case MsgType::mqtt:
        {
            String cmd = obj["params"]["cmd"];
            String file = obj["params"]["file"];
            String var = obj["params"]["var"];
            /* 状态码处理 */
            if (cmd.length() > 0) {
                t_remote_ac_status ac_stauts = getACState(file);
                if (cmd.equals("mode")) {
                    if (var.equals("off")) {
                        ac_stauts.ac_power = AC_POWER_OFF;
                    } else {
                        ac_stauts.ac_power = AC_POWER_ON;
                    }
                    if (var.equals("cool")) ac_stauts.ac_mode = (t_ac_mode)0;
                    if (var.equals("heat")) ac_stauts.ac_mode = (t_ac_mode)1;
                    if (var.equals("auto")) ac_stauts.ac_mode = (t_ac_mode)2;
                    if (var.equals("fan")) ac_stauts.ac_mode = (t_ac_mode)3;
                    if (var.equals("dry")) ac_stauts.ac_mode = (t_ac_mode)4;
                }
                if (cmd.equals("temperature")) {
                    ac_stauts.ac_temp = (t_ac_temperature)(var.toInt() - 16);
                }
                if (cmd.equals("fan")) {
                    if (var.equals("auto")) ac_stauts.ac_wind_speed = (t_ac_wind_speed)0;
                    if (var.equals("low")) ac_stauts.ac_wind_speed = (t_ac_wind_speed)1;
                    if (var.equals("medium")) ac_stauts.ac_wind_speed = (t_ac_wind_speed)2;
                    if (var.equals("high")) ac_stauts.ac_wind_speed = (t_ac_wind_speed)3;
                }
                if (cmd.equals("swing")) {
                    if (var.equals("on")) ac_stauts.ac_swing = (t_ac_swing)0;
                    if (var.equals("off")) ac_stauts.ac_swing = (t_ac_swing)1;
                }
                if (cmd.equals("set")) {
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
                sendStatus(file, ac_stauts);
            }
        }
        break;

        case MsgType::udp:
        {
            send_msg_doc.clear();

            if (obj["cmd"] == "config")
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

            if (obj["cmd"] == "discovery")
            {
                String chip_id = String(ESP.getChipId(), HEX);
                chip_id.toUpperCase();
                send_msg_doc["cmd"] = "upload";
                send_msg_doc["params"]["ip"] = WiFi.localIP().toString();
                send_msg_doc["params"]["mac"] = chip_id;
                
                if (ConfigData.containsKey("mqtt")) {
                    send_msg_doc["params"]["mqtt"] = ConfigData["mqtt"];
                }

                if (ConfigData.containsKey("send_pin")) {
                    send_msg_doc["params"]["send_pin"] = ConfigData["send_pin"];
                }

                if (ConfigData.containsKey("receive_pin")) {
                    send_msg_doc["params"]["receive_pin"] = ConfigData["receive_pin"];
                }
                if (ConfigData.containsKey("version")) {
                    send_msg_doc["params"]["version"] = ConfigData["version"];
                }

                String ip_string = obj["params"]["ip"];
                IPAddress remote_ip;
                remote_ip.fromString(ip_string);
                sendUDP(&send_msg_doc, remote_ip);
            }

            if (obj["cmd"] == "send")
            {
                JsonObject params = obj["params"];
                if (params.containsKey("status")) {
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
                    String file_name = params["file"];
                    sendStatus(file_name, status);
                }
            }

            if (obj["cmd"] == "update") {
                JsonObject params = obj["params"];
                String url = params["url"];
                otaUpdate(url);
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
    String topic_head = "/IRbaby/" + chip_id + "/state/" + file_name
        + "/";
    String topic = topic_head + "mode";
    String payload;
    const char *mode[5] = {"auto", "cool", "test", "a", "b"};
    int index = (int)status.ac_mode;
    DEBUGLN("mode is");
    DEBUGLN(mode[index]);
    // switch (status.ac_mode) {
    //     case t_ac_mode::AC_MODE_AUTO: payload = "auto";
    //     case t_ac_mode::AC_MODE_COOL: payload = "cool";
    //     case t_ac
    // }
    // mqttPublish(topic, )
}