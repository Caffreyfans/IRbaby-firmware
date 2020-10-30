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

extern uint8_t ir_send_pin;
extern uint8_t ir_receive_pin;
#ifdef USE_RF
extern uint8_t rf315_send_pin;
extern uint8_t rf315_receive_pin;
extern uint8_t rf433_send_pin;
extern uint8_t rf433_receive_pin;
#endif
#endif // IRBABY_GLOBAL_H