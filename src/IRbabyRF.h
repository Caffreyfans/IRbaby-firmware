#ifndef IRBABYRF_H
#define IRBABYRF_H
#include <Arduino.h>
#include "RCSwitch.h"

typedef enum
{
    RF315,
    RF433
} RFTYPE;

void initRF(void);
bool sendRFFile(String file_name);
void sendRFData(unsigned long code, unsigned int length, RFTYPE type);
void recvRF(void);
void disableRF(void);
void enableRF(void);
bool saveRF(String file_name);

extern RCSwitch rf315;
extern RCSwitch rf433;
#endif