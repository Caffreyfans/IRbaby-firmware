#ifndef _DEFINES_H
#define _DEFINES_H

typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;

#define FIRMWARE_VERSION "v0.4" // version name
#define VERSION_CODE 4          // version code

/* reset settings */
#define RESET_PIN 13 // reset pin
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

/* mqtt settings */
#define MQTT_CHECK_INTERVALS 5       // seconds
#define MQTT_CONNECT_WAIT_TIME 20000 // MQTT 连接等待时间
#endif