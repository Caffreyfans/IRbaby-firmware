#ifndef IREASY_MSG_HANDLE_H
#define IREASY_MSG_HANDLE_H

#include <ArduinoJson.h>

typedef enum msgtype
{
    mqtt,
    udp
} MsgType;

bool msgHandle(StaticJsonDocument<1024> *p_recv_msg_doc, MsgType msg_type);

#endif