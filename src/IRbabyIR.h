#ifndef IRBABYIR_H
#define IRBABYIR_H

#include <Arduino.h>
#include <IRsend.h>
#include <IRrecv.h>
#include "../lib/Irext/include/ir_decode.h"

void loadIRPin(uint8_t send_pin, uint8_t recv_pin);
void enableIR();
void disableIR();
void sendStatus(String file_name, t_remote_ac_status status);
bool sendKey(String file_name, int key);
bool sendIR(String file_name);
void recvIR();
bool saveIR(String file_name);
void initAC(String);
#endif // IRBABAYIR_H