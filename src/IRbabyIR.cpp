#include "IRbabyIR.h"
#include "ESP8266HTTPClient.h"
#include "IRbabySerial.h"
#include <FS.h>
#include "IRsend.h"
#include "IRbabyUserSettings.h"

#define DOWNLOAD_PREFIX "http://irext-debug.oss-cn-hangzhou.aliyuncs.com/irda_"
#define DOWNLOAD_SUFFIX ".bin"
#define SAVE_PATH "/bin/"


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
            UINT16 content_length = cache.size();
            if (content_length != 0) {
                UINT8 *content = (UINT8 *)malloc(content_length * sizeof(UINT8));
                cache.seek(0L, fs::SeekSet);
                cache.readBytes((char *)content, content_length);
                ir_binary_open(REMOTE_CATEGORY_AC, 1, content, content_length);
                UINT16 user_data[512];
                UINT16 length = ir_decode(2, user_data, &status, 0);
                ir_close();
                free(content);
                IRBABY_DEBUG.println();
                DEBUGF("length = %d\n", length);
                for (int i = 0; i < length; i++) {
                    IRBABY_DEBUG.printf("%d ", user_data[i]);
                }
                IRBABY_DEBUG.println();
                ir_send->sendRaw(user_data, length, 38);
            } else {
                ERRORF("Open %s is empty\n", save_path.c_str());
            }
        }
        cache.close();
    }
}

void recvRaw() {

}