#ifndef IRBABYRF_H
#define IRBABYRF_H
#include <Arduino.h>

bool sendRF(String file_name);
void recvRF(void);
bool saveRF(String file_name);

#endif