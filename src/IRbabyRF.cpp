#include "IRbabyRF.h"
#include <LittleFS.h>
#include "IRbabySerial.h"
#include "IRbabyGlobal.h"
#include "IRbabyUDP.h"
#include "defines.h"
#include <Ticker.h>
RCSwitch rf315;
RCSwitch rf433;

#define FILE_PRE "/rf/"
unsigned long code_tmp;
unsigned int length_tmp;
RFTYPE rf_type_tmp;

Ticker change_rf_ticker; // change receiver
RFTYPE rf_receiver;
bool rf_record;

static void changeReceiver(void)
{
    static bool flag = false;
    flag = !flag;
    rf_receiver = flag ? RF315 : RF433;
}
void sendRFData(unsigned long code, unsigned int length, RFTYPE type)
{
    if (type == RF315)
    {
        rf315.send(code, length);
    }
    else if (type == RF433)
    {
        rf433.send(code, length);
    }
}

bool sendRFFile(String file_name)
{
    File cache;
    unsigned long code;
    unsigned int length;
    RFTYPE rf_type;
    String file_path = FILE_PRE + file_name;
    if (file_name.equals("test"))
    {
        code = code_tmp;
        length = length_tmp;
        rf_type = rf_type_tmp;
    }
    else if (LittleFS.exists(file_path))
    {
        cache = LittleFS.open(file_path, "r");
        if (!cache)
        {
            return false;
        }
        cache.readBytes((char *)&rf_type, sizeof(rf_type_tmp));
        cache.readBytes((char *)&code, sizeof(code));
        cache.readBytes((char *)&length, sizeof(length));
        DEBUGF("type = %d\n", rf_type);
        DEBUGF("code = %ld\n", code);
        DEBUGF("length = %d\n", length);
        cache.close();
    }
    else
    {
        ERRORF("%s file not exists\n", file_name.c_str());
        return false;
    }
    switch (rf_type)
    {
    case RF315:

        rf315.send(code, length);
        break;

    case RF433:
        rf433.send(code, length);
        break;
    default:
        break;
    }
    return true;
}

void recvRF(void)
{
    if (rf_record)
    {
        if (rf_receiver == RF433)
        {
            rf433.enableReceive(R_433);
            rf315.disableReceive();
        }
        else
        {
            rf315.enableReceive(R_315);
            rf433.disableReceive();
        }
        delay(100);
        if (rf_receiver == RF433 && rf433.available())
        {
            code_tmp = rf433.getReceivedValue();
            length_tmp = rf433.getReceivedBitlength();
            rf_type_tmp = RF433;
            rf433.resetAvailable();
            send_msg_doc.clear();
            send_msg_doc["cmd"] = "record_rt";
            send_msg_doc["params"]["signal"] = "RF433";
            send_msg_doc["params"]["value"] = String(code_tmp);
            sendUDP(&send_msg_doc, remote_ip);
            serializeJsonPretty(send_msg_doc, Serial);
        }

        else if (rf_receiver == RF315 && rf315.available())
        {
            code_tmp = rf315.getReceivedValue();
            length_tmp = rf315.getReceivedBitlength();
            rf_type_tmp = RF315;
            rf315.resetAvailable();
            send_msg_doc.clear();
            send_msg_doc["cmd"] = "record_rt";
            send_msg_doc["params"]["signal"] = "RF315";
            send_msg_doc["params"]["value"] = String(code_tmp);
            sendUDP(&send_msg_doc, remote_ip);
            serializeJsonPretty(send_msg_doc, Serial);
        }
    }
}

bool saveRF(String file_name)
{
    String save_path = FILE_PRE + file_name;
    File cache = LittleFS.open(save_path, "w");
    if (!cache)
    {
        return false;
    }
    cache.write((char *)&rf_type_tmp, sizeof(rf_type_tmp));
    cache.write((char *)&code_tmp, sizeof(code_tmp));
    cache.write((char *)&length_tmp, sizeof(length_tmp));
    cache.close();
    return true;
}

void disableRF(void)
{
    rf_record = false;
    rf315.disableReceive();
    rf433.disableReceive();
}

void enableRF(void)
{
    rf_record = true;
}

void initRF(void)
{
    rf_record = false;
    rf315.enableTransmit(T_315);
    rf433.enableTransmit(T_433);
    change_rf_ticker.attach_ms_scheduled(100, changeReceiver);
}