#include "IRbabyha.h"
#include "IRbabyGlobal.h"
#include "IRbabyMQTT.h"
#include "IRbabySerial.h"

void returnACStatus(String filename, t_remote_ac_status ac_status)
{
    send_msg_doc.clear();
    String chip_id = String(ESP.getChipId(), HEX);
    chip_id.toUpperCase();
    String topic_head = "/IRbaby/" + chip_id + "/state/" + filename + "/";
    const char* mode[] = {"cool", "heat", "auto", "fan_only", "dry"};
    const char* fan[] = {"auto", "low", "medium", "high", "max"};
    const char* swing[] = {"on","off"};
    if (ac_status.ac_power == AC_POWER_OFF)
        mqttPublish(topic_head + "mode", "off");
    else
        mqttPublish(topic_head + "mode", mode[(int)ac_status.ac_mode]);
    mqttPublish(topic_head + "temperature", String((int)ac_status.ac_temp + 16));
    mqttPublish(topic_head + "fan", fan[(int)ac_status.ac_wind_speed]);
    mqttPublish(topic_head + "swing", swing[(int)ac_status.ac_swing]);
}

// 自动注册当前空调
void registAC(String filename, bool flag)
{
    send_msg_doc.clear();
    String chip_id = String(ESP.getChipId(), HEX);
    chip_id.toUpperCase();
    String reg_topic_head = "homeassistant/climate/IRbaby-"+ chip_id + "_climate_" + filename +"/config";
    String reg_content;
    if (flag) {
        send_msg_doc["platform"] = "mqtt";
        send_msg_doc["name"] = filename;

        JsonArray modes = send_msg_doc.createNestedArray("modes");
        modes.add("auto");
        modes.add("heat");
        modes.add("cool");
        modes.add("fan_only");
        modes.add("dry");
        modes.add("off");

        JsonArray swing_modes = send_msg_doc.createNestedArray("swing_modes");
        swing_modes.add("on");
        swing_modes.add("off");
        send_msg_doc["max_temp"] = 30;
        send_msg_doc["min_temp"] = 16;

        JsonArray fan_modes = send_msg_doc.createNestedArray("fan_modes");
        fan_modes.add("auto");
        fan_modes.add("low");
        fan_modes.add("medium");
        fan_modes.add("high");
        String cmd_head_topic = "/IRbaby/" + chip_id + "/send/ir/local/" + filename + "/";
        String state_head_topic = "/IRbaby/" + chip_id + "/state/" + filename + "/";
        send_msg_doc["mode_command_topic"] = cmd_head_topic + "mode";
        send_msg_doc["mode_state_topic"] = state_head_topic + "mode";
        send_msg_doc["temperature_command_topic"] = cmd_head_topic + "temperature";
        send_msg_doc["temperature_state_topic"] = state_head_topic + "temperature";
        send_msg_doc["fan_mode_command_topic"] = cmd_head_topic + "fan";
        send_msg_doc["fan_mode_state_topic"] = state_head_topic + "fan";
        send_msg_doc["swing_mode_command_topic"] = cmd_head_topic + "swing";
        send_msg_doc["swing_mode_state_topic"] = state_head_topic + "swing";
        send_msg_doc["precision"] = 0.1;
        send_msg_doc["unique_id"] = "IRbaby-" + chip_id + "_climate_" + filename;

        JsonObject device = send_msg_doc.createNestedObject("device");
        JsonArray device_identifiers = device.createNestedArray("identifiers");
        device_identifiers.add("IRbaby-" + chip_id);
        device["name"] = "IRbaby-" + chip_id;
        device["manufacturer"] = "espressif";
        device["model"] = "ESP8266";
        device["sw_version"] = "IRbaby " + String(FIRMWARE_VERSION);
    }
    if (flag) {
        serializeJson(send_msg_doc, reg_content);
    }
    DEBUGLN(reg_topic_head);
    mqttPublishRetained(reg_topic_head, reg_content);
}
