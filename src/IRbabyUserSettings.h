#ifndef IRBABY_USER_SETTINGS_H
#define IRBABY_USER_SETTINGS_H
#include <ArduinoJson.h>
#include "IRsend.h"

/* 初始化配置信息 */
bool settingsInit();

/* 保存配置信息 */
bool settingsSave();

/* 清除配置信息 */
void settingsClear();

/* 加载配置信息 */
bool settingsLoad();

extern StaticJsonDocument<1024> ConfigData;

extern IRsend* ir_send;

#endif // IRBABY_USER_SETTINGS_H