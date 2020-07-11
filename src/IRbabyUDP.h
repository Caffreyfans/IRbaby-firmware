#ifndef IRBABY_UDP_H
#define IRBABY_UDP_H
#include "ArduinoJson.h"
#include "ESP8266WiFi.h"

#define UDP_PORT 4210
#define UDP_PACKET_SIZE 255

void udpInit();
char* udpRecive();
uint32_t sendUDP(StaticJsonDocument<1024>* doc, IPAddress ip);
uint32_t returnUDP(StaticJsonDocument<1024>* doc);

extern IPAddress remote_ip;
#endif // IRBABY_UDP_H