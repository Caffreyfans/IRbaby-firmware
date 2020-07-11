/**********************
 * 串口打印设置
**********************/

#ifndef IRBABY_SERIAL_H
#define IRBABY_SERIAL_H

#include <Arduino.h>
#include "defines.h"
#define DEBUGLN(...) \
    {if (LOG_DEBUG) { Serial.printf("DEBUG:\t"); Serial.println(__VA_ARGS__);}} 
#define DEBUGF(...) \
    {if (LOG_DEBUG) { Serial.printf("DEBUG:\t"); Serial.printf(__VA_ARGS__);}} 
#define DEBUG(...)  \
    {if (LOG_DEBUG) { Serial.printf("DEBUG:\t"); Serial.print(__VA_ARGS__);}} 

#define INFOLN(...) \
    {if (LOG_INFO) { Serial.printf("INFO:\t"); Serial.println(__VA_ARGS__);}} 
#define INFOF(...) \
    {if (LOG_INFO) { Serial.printf("INFO:\t"); Serial.printf(__VA_ARGS__);}} 
#define INFO(...)  \
    {if (LOG_INFO) { Serial.printf("INFO:\t"); Serial.print(__VA_ARGS__);}} 

#define ERRORLN(...) \
    {if (LOG_ERROR) { Serial.printf("ERROR:\t"); Serial.println(__VA_ARGS__);}} 
#define ERRORF(...) \
    {if (LOG_ERROR) { Serial.printf("ERROR:\t"); Serial.printf(__VA_ARGS__);}} 
#define ERROR(...)  \
    {if (LOG_ERROR) { Serial.printf("ERROR:\t"); Serial.print(__VA_ARGS__);}} 
#endif // IREASY_SERIAL_H