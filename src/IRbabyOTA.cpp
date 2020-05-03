#include "IRbabyOTA.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>

void update_started()
{
    DEBUGLN("CALLBACK:  HTTP update process started");
}

void update_finished()
{
    DEBUGLN("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total)
{
    DEBUGF("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err)
{
    DEBUGF("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void versionCheck()
{
}

void otaUpdate(String url)
{

    WiFiClient client;
    // Add optional callback notifiers
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        DEBUGF("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        DEBUGLN("HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        DEBUGLN("HTTP_UPDATE_OK");
        break;
    }
}