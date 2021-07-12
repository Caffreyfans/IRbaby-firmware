#include "IRbabyMQTT.h"
#include "PubSubClient.h"
#include "IRbabySerial.h"
#include "ArduinoJson.h"
#include "IRbabyUserSettings.h"
#include "IRbabyMsgHandler.h"
#include "IRbabyGlobal.h"
PubSubClient mqtt_client(wifi_client);

void callback(char *topic, byte *payload, unsigned int length);

void mqttInit()
{
    INFOLN("MQTT Init");
    mqttReconnect();
    mqtt_client.setCallback(callback);
}

/**************************************
 * sub example: /IRbaby/chip_id/set/#
 **************************************/
bool mqttReconnect()
{
    bool flag = false;
    if (ConfigData.containsKey("mqtt"))
    {
        JsonObject mqtt_obj = ConfigData["mqtt"];
        const char *host = mqtt_obj["host"];
        int port = mqtt_obj["port"];
        const char *user = mqtt_obj["user"];
        const char *password = mqtt_obj["password"];
        if (host && port)
        {
            mqtt_client.setServer(host, port);
            String chip_id = String(ESP.getChipId(), HEX);
            chip_id.toUpperCase();
            DEBUGF("Trying to connect %s:%d\n", host, port);
            if (mqtt_client.connect(chip_id.c_str(), user,
                                    password))
            {
                String sub_topic = String("/IRbaby/") +
                                   chip_id + String("/send/#");
                DEBUGF("MQTT subscribe %s\n", sub_topic.c_str());
                mqtt_client.subscribe(sub_topic.c_str());
                flag = true;
            }
        }
        INFOF("MQTT state rc = %d\n", mqtt_client.state());
    }
    delay(1000);
    return flag;
}

void mqttDisconnect()
{
    mqtt_client.disconnect();
}

void callback(char *topic, byte *payload, unsigned int length)
{
    mqtt_msg_doc.clear();

    String payload_str = "";
    for (uint32_t i = 0; i < length; i++)
        payload_str += (char)payload[i];
    String topic_str(topic);
    uint8_t index = 0;
    String option;
    String func;
    do
    {
        int divsion = topic_str.lastIndexOf("/");
        String tmp = topic_str.substring(divsion + 1, -1);
        topic_str = topic_str.substring(0, divsion);
        switch (index++)
        {
        case 0:
            func = tmp;
            break;
        case 1:
            mqtt_msg_doc["params"]["file"] = tmp;
            break;
        case 2:
            mqtt_msg_doc["params"]["type"] = tmp;
            option = tmp;
            break;
        case 3:
            mqtt_msg_doc["params"]["signal"] = tmp;
            break;
        case 4:
            mqtt_msg_doc["cmd"] = tmp;
        default:
            break;
        }
    } while (topic_str.lastIndexOf("/") > 0);
    mqtt_msg_doc["params"][option][func] = payload_str;
    msgHandle(&mqtt_msg_doc, MsgType::mqtt);
}

bool mqttConnected()
{
    return mqtt_client.connected();
}

void mqttLoop()
{
    mqtt_client.loop();
}

void mqttPublish(String topic, String payload)
{
    mqtt_client.publish(topic.c_str(), payload.c_str());
}

void mqttPublishRetained(String topic, String payload)
{
    mqtt_client.publish(topic.c_str(), payload.c_str(), true);
}

void mqttCheck()
{
    if (!mqttConnected())
    {
        DEBUGLN("MQTT disconnect, try to reconnect");
        mqtt_client.disconnect();
        mqttReconnect();
        led.Blink(500, 500);
    } else {
        led.On();
    }
}