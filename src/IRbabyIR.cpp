#include "IRbabyIR.h"
#include "ESP8266HTTPClient.h"
#include "IRbabySerial.h"
#include "IRbabyUDP.h"
#include "IRbabyUserSettings.h"
#include <LittleFS.h>
#include "IRbabyGlobal.h"

#define DOWNLOAD_PREFIX "http://irext-debug.oss-cn-hangzhou.aliyuncs.com/irda_"
#define DOWNLOAD_SUFFIX ".bin"
#define SAVE_PATH "/bin/"

decode_results results; // Somewhere to store the results
IRsend *ir_send = NULL;
IRrecv *ir_recv = NULL;

void initAC(String);
bool saveAC(String, t_remote_ac_status);
bool saveSignal();

void downLoadFile(String file)
{
    HTTPClient http_client;
    String download_url = DOWNLOAD_PREFIX + file + DOWNLOAD_SUFFIX;
    String save_path = SAVE_PATH + file;
    File cache = LittleFS.open(save_path, "w");
    if (cache)
    {
        http_client.begin(wifi_client, download_url);
        if (HTTP_CODE_OK == http_client.GET())
        {
            http_client.writeToStream(&cache);
            DEBUGF("Download %s success\n", file.c_str());
        }
        else
        {
            LittleFS.remove(save_path);
            DEBUGF("Download %s failed\n", file.c_str());
        }
    }
    else
        ERRORLN("Don't have enough file zoom");
    cache.close();
    http_client.end();
}

bool sendRaw(String file_name)
{
    String save_path = SAVE_PATH + file_name;
    if (LittleFS.exists(save_path))
    {
        File cache = LittleFS.open(save_path, "r");
        if (!cache)
        {
            ERRORF("Failed to open %s", save_path.c_str());
            return false;
        }
        Serial.println();
        uint16_t *data_buffer = (uint16_t *)malloc(sizeof(uint16_t) * 512);
        uint16_t length = cache.size() / 2;
        memset(data_buffer, 0x0, 512);
        INFOF("file size = %d\n", cache.size());
        INFOLN();
        cache.readBytes((char *)data_buffer, cache.size());
        // for (size_t i = 0; i < length; i++)
        //     Serial.printf("%d ", *(data_buffer + i));
        // INFOLN();
        ir_recv->disableIRIn();
        ir_send->sendRaw(data_buffer, length, 38);
        ir_recv->enableIRIn();
        free(data_buffer);
        cache.close();
        return true;
    }
    return false;
}

void sendStatus(String file, t_remote_ac_status status)
{
    String save_path = SAVE_PATH + file;
    if (!LittleFS.exists(save_path))
        downLoadFile(file);

    if (LittleFS.exists(save_path))
    {
        File cache = LittleFS.open(save_path, "r");
        if (cache)
        {
            UINT16 content_size = cache.size();
            DEBUGF("content size = %d\n", content_size);

            if (content_size != 0)
            {
                UINT8 *content = (UINT8 *)malloc(content_size * sizeof(UINT8));
                cache.seek(0L, fs::SeekSet);
                cache.readBytes((char *)content, content_size);
                ir_binary_open(REMOTE_CATEGORY_AC, 1, content, content_size);
                UINT16 *user_data = (UINT16 *)malloc(1024 * sizeof(UINT16));
                UINT16 data_length = ir_decode(0, user_data, &status, FALSE);

                DEBUGF("data_length = %d\n", data_length);
                if (LOG_DEBUG)
                {
                    for (int i = 0; i < data_length; i++)
                        Serial.printf("%d ", *(user_data + i));
                    Serial.println();
                }
                ir_recv->disableIRIn();
                ir_send->sendRaw(user_data, data_length, 38);
                ir_recv->enableIRIn();
                ir_close();
                free(user_data);
                free(content);
            }
            else
                ERRORF("Open %s is empty\n", save_path.c_str());
        }
        cache.close();
    }
    saveAC(file, status);
}

void recvRaw()
{
    if (ir_recv->decode(&results))
    {
        DEBUGF("raw length = %d\n", results.rawlen - 1);
        String raw_data;
        raw_data += "raw length = ";
        raw_data += String(results.rawlen - 1) + "\n";
        for (int i = 1; i < results.rawlen; i++)
            raw_data += String(*(results.rawbuf + i) * kRawTick) + " ";
        ir_recv->resume();
        send_msg_doc.clear();
        send_msg_doc["cmd"] = "record_rt";
        send_msg_doc["params"] = raw_data.c_str();
        DEBUGLN(raw_data.c_str());
        sendUDP(&send_msg_doc, remote_ip);
        saveSignal();
    }
}

bool saveRaw(String file_name)
{
    String save_path = SAVE_PATH;
    save_path += file_name;
    return LittleFS.rename("/bin/test", save_path);
}

bool saveSignal()
{
    String save_path = SAVE_PATH;
    save_path += "test";
    DEBUGF("save raw data as %s\n", save_path.c_str());
    File cache = LittleFS.open(save_path, "w");
    if (!cache)
    {
        ERRORLN("Failed to create file");
        return false;
    }
    for (size_t i = 0; i < results.rawlen; i++)
        *(results.rawbuf + i) = *(results.rawbuf + i) * kRawTick;
    cache.write((char *)(results.rawbuf + 1), (results.rawlen - 1) * 2);
    cache.close();
    return true;
}
void initAC(String file)
{
    ACStatus[file]["power"] = 0;
    ACStatus[file]["temperature"] = 8;
    ACStatus[file]["mode"] = 2;
    ACStatus[file]["swing"] = 0;
    ACStatus[file]["speed"] = 0;
}

t_remote_ac_status getACState(String file)
{
    if (!ACStatus.containsKey(file))
        initAC(file);
    t_remote_ac_status status;
    int power = ACStatus[file]["power"];
    int temperature = ACStatus[file]["temperature"];
    int mode = ACStatus[file]["mode"];
    int swing = ACStatus[file]["swing"];
    int wind_speed = ACStatus[file]["speed"];
    status.ac_power = (t_ac_power)power;
    status.ac_temp = (t_ac_temperature)temperature;
    status.ac_mode = (t_ac_mode)mode;
    status.ac_swing = (t_ac_swing)swing;
    status.ac_wind_speed = (t_ac_wind_speed)wind_speed;
    return status;
}

bool saveAC(String file, t_remote_ac_status status)
{
    bool ret = false;
    ACStatus[file]["power"] = status.ac_power;
    ACStatus[file]["temperature"] = status.ac_temp;
    ACStatus[file]["mode"] = status.ac_mode;
    ACStatus[file]["swing"] = status.ac_swing;
    ACStatus[file]["speed"] = status.ac_wind_speed;
    File cache = LittleFS.open("/acstatus", "w");
    if (cache && (serializeJson(ACStatus, cache) == 0))
        ret = true;
    cache.close();
    return ret;
}

bool sendKey(String file_name, int key)
{
    String save_path = SAVE_PATH;
    save_path += file_name;
    if (LittleFS.exists(save_path))
    {
        File cache = LittleFS.open(save_path, "r");
        if (cache)
        {
            UINT16 content_size = cache.size();
            DEBUGF("content size = %d\n", content_size);

            if (content_size != 0)
            {
                UINT8 *content = (UINT8 *)malloc(content_size * sizeof(UINT8));
                cache.seek(0L, fs::SeekSet);
                cache.readBytes((char *)content, content_size);
                ir_binary_open(2, 1, content, content_size);
                UINT16 *user_data = (UINT16 *)malloc(1024 * sizeof(UINT16));
                UINT16 data_length = ir_decode(0, user_data, NULL, FALSE);

                DEBUGF("data_length = %d\n", data_length);
                if (LOG_DEBUG)
                {
                    for (int i = 0; i < data_length; i++)
                        Serial.printf("%d ", *(user_data + i));
                    Serial.println();
                }
                ir_recv->disableIRIn();
                ir_send->sendRaw(user_data, data_length, 38);
                ir_recv->enableIRIn();
                ir_close();
                free(user_data);
                free(content);
            }
            else
                ERRORF("Open %s is empty\n", save_path.c_str());
        }
        cache.close();
    }
    return true;
}