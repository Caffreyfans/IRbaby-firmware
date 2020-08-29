#ifndef IRBABY_MQTT_H
#define IRBABY_MQTT_H
#include <WString.h>
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

/* MQTT 信息发送 */
void mqttPublishRetained(String topic, String payload);

/* MQTT 连接检查 */
void mqttCheck();
#endif // IRBABY_MQTT_H