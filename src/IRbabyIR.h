#ifndef IRBABYIR_H
#define IRBABYIR_H

#include <Arduino.h>
#include <IRsend.h>
#include <IRrecv.h>
#include "../lib/Irext/include/ir_decode.h"
void sendStatus(String file_name, t_remote_ac_status status);
bool sendRaw(String file_name);
void recvRaw();
bool saveRaw(String file_name);
t_remote_ac_status getACState(String file);

extern IRsend* ir_send;
extern IRrecv* ir_recv;
#endif // IRBABAYIR_H