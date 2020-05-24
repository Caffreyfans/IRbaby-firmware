#include "IRbabyIR.h"
#include "ESP8266HTTPClient.h"
#include "IRbabySerial.h"
#include <FS.h>
#include "IRbabyUserSettings.h"

#define DOWNLOAD_PREFIX "http://irext-debug.oss-cn-hangzhou.aliyuncs.com/irda_"
#define DOWNLOAD_SUFFIX ".bin"
#define SAVE_PATH "/bin/"

StaticJsonDocument<1024> ac_status_doc;
decode_results results;  // Somewhere to store the results

void initAC(String file);
void saveAC(String file, t_remote_ac_status status);

void downLoadFile(String file) {
    HTTPClient http_client;
    String download_url = DOWNLOAD_PREFIX + file + DOWNLOAD_SUFFIX;
    String save_path = SAVE_PATH + file;



    File cache = SPIFFS.open(save_path, "w");
    if (cache) {
        http_client.begin(download_url);
        if (HTTP_CODE_OK == http_client.GET()) {
            DEBUGF("Download %s success\n", file.c_str());
            http_client.writeToStream(&cache);
        } else {
            SPIFFS.remove(save_path);
            DEBUGF("Download %s failed\n", file.c_str());
        }
    } else {
        ERRORLN("Don't have enough file zoom");
    }
    cache.close();
    http_client.end();
}

void sendStatus(String file, t_remote_ac_status status) {
    String save_path = SAVE_PATH + file;
    if (!SPIFFS.exists(save_path)) {
        downLoadFile(file);
    }

    if (SPIFFS.exists(save_path)) {
        File cache = SPIFFS.open(save_path, "r");
        if (cache) {
            UINT16 content_size = cache.size();
            DEBUGF("content size = %d\n", content_size);

            if (content_size != 0) {
                UINT8 *content = (UINT8 *)malloc(content_size * sizeof(UINT8));
                cache.seek(0L, fs::SeekSet);
                cache.readBytes((char *)content, content_size);
                ir_binary_open(REMOTE_CATEGORY_AC, 1, content, content_size);
                UINT16 *user_data = (UINT16 *)malloc(1024 * sizeof(UINT16));
                UINT16 data_length = ir_decode(0, user_data, &status, FALSE);

                IRBABY_DEBUG.println();
                DEBUGF("data_length = %d\n", data_length);
                for (int i = 0; i < data_length; i++) {
                    IRBABY_DEBUG.printf("%d ", *(user_data + i));
                }
                IRBABY_DEBUG.println();
                ir_send->sendRaw(user_data, data_length, 38);
                ir_close();
                free(user_data);
                free(content);
            } else {
                ERRORF("Open %s is empty\n", save_path.c_str());
            }
        }
        cache.close();
    }
    saveAC(file, status);
}

void recvRaw() {
    if (ir_recv->decode(&results)) {
        DEBUGF("raw length = %d\n", results.rawlen - 1);
        for (int i = 1; i < results.rawlen; i++) {
            IRBABY_DEBUG.printf("%d ", *(results.rawbuf + i) * 2);
        }
        IRBABY_DEBUG.println();
        saveRaw();
        ir_recv->resume();
    }    
}

bool saveRaw() {
    DEBUGLN("try to save data");
    File cache = SPIFFS.open("/raw/test", "w");
    if (!cache)
    {
        ERRORLN("Failed to create file");
        return false;
    }   
    cache.write((char *)(results.rawbuf + 1), 2 * (results.rawlen - 1));
    cache.close();
    cache = SPIFFS.open("/raw/test", "r");
    if (!cache) {
        ERRORLN("Failed to open test");
        return false;
    }
    uint16_t* data_buffer = (uint16_t *)malloc(sizeof(uint16_t) * 512);
    memset(data_buffer, 0, 512);
    DEBUGF("\nfile size = %d\n", cache.size());
    cache.readBytes((char *)data_buffer, cache.size());
    for (int i = 0; i < cache.size() / 2; i++) {
        IRBABY_DEBUG.printf("%d ", *(data_buffer + i) * 2);
    }
    free(data_buffer);
    cache.close();
    return true;
}

void initAC(String file) {
    ac_status_doc[file]["power"] = 0;
    ac_status_doc[file]["temperature"] = 8;
    ac_status_doc[file]["mode"] = 2;
    ac_status_doc[file]["swing"] = 0;
    ac_status_doc[file]["speed"] = 0;
}

t_remote_ac_status getACState(String file) {
    if (!ac_status_doc.containsKey(file)) {
        initAC(file);
    }
    t_remote_ac_status status;
    int power = ac_status_doc[file]["power"];
    int temperature = ac_status_doc[file]["temperature"];
    int mode = ac_status_doc[file]["mode"];
    int swing = ac_status_doc[file]["swing"];
    int wind_speed = ac_status_doc[file]["speed"];
    status.ac_power = (t_ac_power)power;
    status.ac_temp = (t_ac_temperature)temperature;
    status.ac_mode = (t_ac_mode)mode;
    status.ac_swing = (t_ac_swing)swing;
    status.ac_wind_speed = (t_ac_wind_speed)wind_speed;
    return status;
}

void saveAC(String file, t_remote_ac_status status) {
    ac_status_doc[file]["power"] = status.ac_power;
    ac_status_doc[file]["temperature"] = status.ac_temp;
    ac_status_doc[file]["mode"] = status.ac_mode;
    ac_status_doc[file]["swing"] = status.ac_swing;
    ac_status_doc[file]["speed"] = status.ac_wind_speed;
}