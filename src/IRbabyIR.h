#ifndef IRBABYIR_H
#define IRBABYIR_H

#include <Arduino.h>
#include "../lib/Irext/include/ir_decode.h"
void sendRaw(String file);
void sendStatus(String file, t_remote_ac_status status);
void recvRaw();
#endif // IRBABAYIR_H