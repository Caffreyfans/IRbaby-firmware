// IRbaby - github.com/Caffreyfans/IRbaby
// Copyright Miao Yi 2019-2020
// MIT License

#ifndef _DEFINES_H
#define _DEFINES_H

typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;

#define FIRMWARE_VERSION "v0.6" // version name
#define VERSION_CODE 6          // version code

/* log settings */
#define BAUD_RATE 115200
#ifndef LOG_DEBUG
#define LOG_DEBUG false
#endif
#ifndef LOG_INFO
#define LOG_INFO true
#endif
#ifndef LOG_ERROR
#define LOG_ERROR true
#endif

/* ----------------- user settings ----------------- */
/* mqtt settings */
#define MQTT_CHECK_INTERVALS 10      // seconds
#define MQTT_CONNECT_WAIT_TIME 20000 // MQTT 连接等待时间

/* receive disable */
#define DISABLE_SIGNAL_INTERVALS 600 // seconds

// uncomment below to enable RF
// #define USE_RF

// uncomment below to enable upload board info to remote server
// #define USE_INFO_UPLOAD

/* ----------------- default pin setting --------------- */
/* reset pin */
#define RESET_PIN 2

/* 315 RF pin */
#define T_315 5
#define R_315 4

/* 433 RF pin */
#define T_433 14
#define R_433 12

/* IR pin */
#define T_IR 14
#define R_IR 12

#endif  // _DEFINES_H
