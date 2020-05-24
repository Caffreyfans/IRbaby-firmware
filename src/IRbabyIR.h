#ifndef IRBABYIR_H
#define IRBABYIR_H

#include <Arduino.h>
#include "../lib/Irext/include/ir_decode.h"
void sendStatus(String file_name, t_remote_ac_status status);
void sendRaw(String file);
void recvRaw();
bool saveRaw();
t_remote_ac_status getACState(String file);
#endif // IRBABAYIR_H