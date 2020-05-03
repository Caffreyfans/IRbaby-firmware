#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "WiFiManager.h"
#include <ESP8266httpUpdate.h>
#include "IRbabyUDP.h"
#include "IRbabyOTA.h"
#include "IRbabyMQTT.h"
#include "IRbabySerial.h"
#include "IRbabyMsgHandler.h"
#include "IRbabyUserSettings.h"
#include "ESP8266HTTPClient.h"

const byte reset_pin = D6; // 复位键
void ICACHE_RAM_ATTR resetHandle(); // 中断函数
WiFiManager wifi_manager;
uint32_t last_system_time = millis();
uint32_t system_time;

void registerDevice();

void setup()
{
    Serial.begin(115200);
    pinMode(reset_pin, INPUT_PULLUP);
    digitalWrite(reset_pin, HIGH);
    attachInterrupt(digitalPinToInterrupt(reset_pin), resetHandle, ONLOW);
    LOGLN();
    LOGLN();
    LOGLN(" ___________ _           _           ");
    LOGLN("|_   _| ___ \\ |         | |          ");
    LOGLN("  | | | |_/ / |__   __ _| |__  _   _ ");
    LOGLN("  | | |    /| '_ \\ / _` | '_ \\| | | |");
    LOGLN(" _| |_| |\\ \\| |_) | (_| | |_) | |_| |");
    LOGLN(" \\___/\\_| \\_|_.__/ \\__,_|_.__/ \\__, |");
    LOGLN("                                __/ |");
    LOGLN("                               |___/ ");
    wifi_manager.autoConnect();

    SPIFFS.begin(); // 文件管理启动
    settingsLoad(); // 加载配置信息

    udpInit();  // udp 初始化
    mqttInit(); // mqtt 初始化

    registerDevice();
}

void loop()
{
    /* UDP 报文接受处理 */
    char *msg = udpRecive();
    if (msg)
    {
        recv_msg_doc.clear();
        DeserializationError error = deserializeJson(recv_msg_doc, msg);
        if (error)
        {
            ERRORLN("Failed to parse udp message");
        }
        msgHandle(&recv_msg_doc, MsgType::udp);
    }

    /* MQTT 连接状态检查 */
    system_time = millis();
    if (system_time - last_system_time > MQTT_RECONNECT_INTERVAL)
    {
        DEBUGLN("Check mqtt connection status");
        if (!mqttConnected())
        {
            DEBUGLN("MQTT disconnect, try to reconnect");
            mqttDisconnect();
            mqttReconnect();
        } else {
            DEBUGLN("MQTT connected");
        }
        last_system_time = system_time;
    }

    /* 接收 MQTT 消息 */
    mqttLoop();
}

void resetHandle()
{
    static unsigned long last_interrupt_time = millis();
    unsigned long interrupt_time = millis();
    static unsigned long start_time = millis();
    unsigned long end_time = millis();
    if (interrupt_time - last_interrupt_time > 10)
    {
        start_time = millis();
    }
    last_interrupt_time = interrupt_time;
    if (end_time - start_time > 3000)
    {
        settingsClear();
    }
}

void registerDevice() {
    HTTPClient http;
    String head = "http://api.ipify.org/?format=json";
    http.begin(head);
    int http_code = http.GET();
    String ip = http.getString();
    StaticJsonDocument<128> ip_json;
    deserializeJson(ip_json, ip);
    JsonObject ip_obj = ip_json.as<JsonObject>();
    http.end();
    HTTPClient http2;
    StaticJsonDocument<128> body_json;
    String chip_id = String(ESP.getChipId(), HEX);
    chip_id.toUpperCase();
    head = "http://playground.devicehive.com/api/rest/device/";
    head += chip_id;
    http2.begin(head);
    http2.addHeader("Content-Type", "application/json");
    http2.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiJ9.eyJwYXlsb2FkIjp7ImEiOlsyLDMsNCw1LDYsNyw4LDksMTAsMTEsMTIsMTUsMTYsMTddLCJlIjoxNzQzNDM2ODAwMDAwLCJ0IjoxLCJ1Ijo2NjM1LCJuIjpbIjY1NDIiXSwiZHQiOlsiKiJdfX0.WyyxNr2OD5pvBSxMq84NZh6TkNnFZe_PXenkrUkRSiw");
    body_json["name"] = chip_id;
    body_json["networkId"] = "6542";
    body_json["data"] = ip_obj;
    String body = body_json.as<String>();
    DEBUGLN(body);
    http_code = http2.PUT(body);
    http2.end();   
}