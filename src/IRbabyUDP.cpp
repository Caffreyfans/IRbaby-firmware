#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include "IRbabyUDP.h"
#include "IRbabySerial.h"

WiFiUDP UDP;

char incomingPacket[UDP_PACKET_SIZE]; // buffer for incoming packets

void udpInit()
{
    UDP.begin(UDP_PORT);
    DEBUGF("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), UDP_PORT);
}

char *udpRecive()
{
    int packetSize = UDP.parsePacket();
    if (packetSize)
    {
        /* receive incoming UDP packets */
        DEBUGF("Received %d bytes from %s, port %d\n", packetSize, UDP.remoteIP().toString().c_str(), UDP.remotePort());
        int len = UDP.read(incomingPacket, 255);
        if (len > 0)
        {
            incomingPacket[len] = 0;
        }
        return incomingPacket;
    }
    return nullptr;
}

uint32_t sendUDP(StaticJsonDocument<1024>* udp_msg_doc, IPAddress ip)
{
    UDP.beginPacket(ip, UDP_PORT);
    serializeJson(*udp_msg_doc, UDP);
    return UDP.endPacket();
}

uint32_t returnUDP(StaticJsonDocument<1024>* udp_msg_doc)
{
    UDP.beginPacket(UDP.remoteIP(), UDP_PORT);
    serializeJson(*udp_msg_doc, UDP);
    return UDP.endPacket();
}