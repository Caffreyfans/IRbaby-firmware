#ifndef IRBABY_USER_SETTINGS_H
#define IRBABY_USER_SETTINGS_H
#include <ArduinoJson.h>
#include "../lib/Irext/include/ir_ac_control.h"
/* 保存配置信息 */
bool settingsSave();

/* 清除配置信息 */
void settingsClear();

/* 加载配置信息 */
bool settingsLoad();

bool saveACStatus(String, t_remote_ac_status);
t_remote_ac_status getACState(String file);

extern StaticJsonDocument<1024> ConfigData;
extern StaticJsonDocument<1024> ACStatus;
#endif // IRBABY_USER_SETTINGS_H