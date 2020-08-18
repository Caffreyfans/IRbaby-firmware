/**
 *
 * Copyright (c) 2020 IRbaby
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Ticker.h>
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include "IRbabyIR.h"
#include "IRbabyUDP.h"
#include "IRbabyOTA.h"
#include "IRbabyMQTT.h"
#include "IRbabyMsgHandler.h"
#include "defines.h"
#include "IRbabyGlobal.h"
#include "IRbabyUserSettings.h"
#include "IRbabyRF.h"

void uploadIP();              // device info upload to devicehive
void ICACHE_RAM_ATTR resetHandle(); // interrupt handle
Ticker mqtt_check;                  // MQTT check timer
Ticker disable_ir;                  // disable IR receive
Ticker disable_rf;                  // disable RF receive

void setup()
{
    if (LOG_DEBUG || LOG_ERROR || LOG_INFO)
        Serial.begin(BAUD_RATE);
    pinMode(RESET_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RESET_PIN), resetHandle, ONLOW);
    INFOLN();
    INFOLN("8888888 8888888b.  888               888               ");
    INFOLN("  888   888   Y88b 888               888               ");
    INFOLN("  888   888    888 888               888               ");
    INFOLN("  888   888   d88P 88888b.   8888b.  88888b.  888  888 ");
    INFOLN("  888   8888888P   888  88b      88b 888  88b 888  888 ");
    INFOLN("  888   888 T88b   888  888 .d888888 888  888 888  888 ");
    INFOLN("  888   888  T88b  888 d88P 888  888 888 d88P Y88b 888 ");
    INFOLN("8888888 888   T88b 88888P    Y888888 88888P     Y88888 ");
    INFOLN("                                              Y8b d88P ");
    INFOLN("                                                 Y88P  ");

    wifi_manager.autoConnect();

    settingsLoad();  // load user settings form fs

    udpInit();  // udp init
    mqttInit(); // mqtt init
#ifdef USE_RF
    initRF();   // RF init
#endif
    loadIRPin(ConfigData["pin"]["ir_send"], ConfigData["pin"]["ir_receive"]);;   // IR init
#ifdef USE_INFO_UPLOAD
    uploadIP();
#endif
    mqtt_check.attach_scheduled(MQTT_CHECK_INTERVALS, mqttCheck);
    disable_ir.attach_scheduled(DISABLE_SIGNAL_INTERVALS, disableIR);
    disable_rf.attach_scheduled(DISABLE_SIGNAL_INTERVALS, disableRF);
}

void loop()
{
    /* IR receive */
    recvIR();
#ifdef USE_RF
    /* RF receive */
    recvRF();
#endif
    /* UDP receive and handle */
    char *msg = udpRecive();
    if (msg) {
        udp_msg_doc.clear();
        DeserializationError error = deserializeJson(udp_msg_doc, msg);
        if (error)
            ERRORLN("Failed to parse udp message");
        msgHandle(&udp_msg_doc, MsgType::udp);
    }

    /* mqtt loop */
    mqttLoop();
    yield();
}

void resetHandle()
{
    static unsigned long last_interrupt_time = millis();
    unsigned long interrupt_time = millis();
    static unsigned long start_time = millis();
    unsigned long end_time = millis();
    if (interrupt_time - last_interrupt_time > 10)
        start_time = millis();
    last_interrupt_time = interrupt_time;
    if (end_time - start_time > 3000)
        settingsClear();
}

// only upload chip id
void uploadIP()
{
    HTTPClient http;
    StaticJsonDocument<128> body_json;
    String chip_id = String(ESP.getChipId(), HEX);
    chip_id.toUpperCase();
    String head = "http://playground.devicehive.com/api/rest/device/";
    head += chip_id;
    http.begin(wifi_client, head);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiJ9.eyJwYXlsb2FkIjp7ImEiOlsyLDMsNCw1LDYsNyw4LDksMTAsMTEsMTIsMTUsMTYsMTddLCJlIjoxNzQzNDM2ODAwMDAwLCJ0IjoxLCJ1Ijo2NjM1LCJuIjpbIjY1NDIiXSwiZHQiOlsiKiJdfX0.WyyxNr2OD5pvBSxMq84NZh6TkNnFZe_PXenkrUkRSiw");
    body_json["name"] = chip_id;
    body_json["networkId"] = "6542";
    String body = body_json.as<String>();
    INFOF("update %s to devicehive\n", body.c_str());
    http.PUT(body);
    http.end();
}