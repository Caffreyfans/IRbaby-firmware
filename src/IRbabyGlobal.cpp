#include "IRbabyGlobal.h"

StaticJsonDocument<1024> recv_msg_doc;
StaticJsonDocument<1024> send_msg_doc;
StaticJsonDocument<1024> udp_msg_doc;
StaticJsonDocument<1024> mqtt_msg_doc;

WiFiManager wifi_manager;
WiFiClient wifi_client;

