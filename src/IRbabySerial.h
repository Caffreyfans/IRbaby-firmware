/**********************
 * 串口打印设置
**********************/

#ifndef IRBABY_SERIAL_H
#define IRBABY_SERIAL_H

#include <Arduino.h>

#define IRBABY_DEBUG Serial
#define IRBABY_LOG Serial
#define IRBABY_ERROR Serial

#ifdef IRBABY_DEBUG
#define DEBUGLN(...)               \
    IRBABY_DEBUG.print("DEBUG: "); \
    IRBABY_DEBUG.println(__VA_ARGS__)
#define DEBUGF(...)                \
    IRBABY_DEBUG.print("DEBUG: "); \
    IRBABY_DEBUG.printf(__VA_ARGS__)
#define DEBUG(...)                 \
    IRBABY_DEBUG.print("DEBUG: "); \
    IRBABY_DEBUG.print(__VA_ARGS__)
#endif // IRBABY_DEBUG

#ifdef IRBABY_LOG
#define LOGLN(...)             \
    IRBABY_LOG.print("LOG: "); \
    IRBABY_LOG.println(__VA_ARGS__)
#define LOGF(...)              \
    IRBABY_LOG.print("LOG: "); \
    IRBABY_LOG.printf(__VA_ARGS__)
#define LOG(...)               \
    IRBABY_LOG.print("LOG: "); \
    IRBABY_LOG.print(__VA_ARGS__)
#endif // IRBABY_LOG

#ifdef IRBABY_ERROR
#define ERRORLN(...)               \
    IRBABY_ERROR.print("ERROR: "); \
    IRBABY_ERROR.println(__VA_ARGS__)
#define ERRORF(...)                \
    IRBABY_ERROR.print("ERROR: "); \
    IRBABY_ERROR.printf(__VA_ARGS__)
#define ERROR(...)                 \
    IRBABY_ERROR.print("ERROR: "); \
    IRBABY_ERROR.print(__VA_ARGS__)
#endif // IRBABY_ERROR

#endif // IREASY_SERIAL_H