#ifndef IRBABY_MQTT_H
#define IRBABY_MQTT_H

#include "IRbabyGlobal.h"

#define MQTT_CONNECT_WAIT_TIME 20000  // MQTT 连接等待时间
#define MQTT_RECONNECT_INTERVAL 60000 // MQTT 重连间断时间

/* MQTT 初始化 */
void mqttInit();

/* MQTT 重连 */
bool mqttReconnect();

/* MQTT 断开连接 */
void mqttDisconnect();

/* MQTT 请求连接 */
bool mqttConnected();

/* MQTT 接收循环 */
void mqttLoop();

/* MQTT 信息发送 */
void mqttPublish(String topic, String payload);

#endif // IRBABY_MQTT_H