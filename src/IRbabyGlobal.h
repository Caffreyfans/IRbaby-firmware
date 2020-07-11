#ifndef IRBABY_GLOBAL_H
#define IRBABY_GLOBAL_H

#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
/* goable json variable */
extern StaticJsonDocument<1024> recv_msg_doc;
extern StaticJsonDocument<1024> send_msg_doc;
extern StaticJsonDocument<1024> udp_msg_doc;
extern StaticJsonDocument<1024> mqtt_msg_doc;

extern WiFiManager wifi_manager;
extern WiFiClient wifi_client;

#endif // IRBABY_GLOBAL_H