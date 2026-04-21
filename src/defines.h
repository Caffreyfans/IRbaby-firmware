/*
 * @Author: Caffreyfans
 * @Date: 2021-07-06 20:59:02
 * @LastEditTime: 2021-07-12 23:47:26
 * @Description: 
 */ 

#ifndef _DEFINES_H
#define _DEFINES_H

#include <stdint.h>

#define FIRMWARE_VERSION "v1.1" // version name
#define VERSION_CODE 11          // version code

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
#define MQTT_CHECK_INTERVALS 15      // seconds
#define MQTT_CONNECT_WAIT_TIME 20000 // MQTT 连接等待时间

/* receive disable */
#define DISABLE_SIGNAL_INTERVALS 600 // seconds

#define SAVE_DATA_INTERVALS 300 // seconds

// uncomment below to enable RF
// #define USE_RF

// uncomment below to enable upload chip id to remote server
#define USE_INFO_UPLOAD

// uncomment below to enable binary sensor
#ifdef USE_SENSOR
#define SENSOR_UPLOAD_INTERVAL      10  // seconds
#endif // USE_SENSOR
/* ----------------- default pin setting --------------- */
/* reset pin */
#define RESET_PIN 0
#define LED_PIN   2
/* 315 RF pin */
#define T_315 5
#define R_315 4

/* 433 RF pin */
#define T_433 14
#define R_433 12

/* IR pin */
#define T_IR 14
#define R_IR 12

/* Sensor Pin*/
#define SENSOR_PIN 0

/* Platform-independent chip ID helper */
#ifdef ESP32
// Derive a 32-bit chip ID from the lower 4 bytes of the 48-bit eFuse MAC address
inline uint32_t getChipId() {
    uint32_t chipId = 0;
    for (int i = 0; i < 32; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (32 - i)) & 0xff) << i;
    }
    return chipId;
}
#else
inline uint32_t getChipId() {
    return ESP.getChipId();
}
#endif

#endif  // _DEFINES_H
