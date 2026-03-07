#ifndef SERVER_H
#define SERVER_H

#include <WiFi.h>
#include "controller.h"

extern struct CONTROLLER_STATUS iemSta;
extern struct CONTROLLER_SETTINGS iemConf;

#define MQTT_BROKER "mqtt.example.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "controller/status"
#define MQTT_SUBSCRIBE "controller/settings" 

#define WIFI_STA_SSID "VARSHA F1 2G"
#define WIFI_STA_PASSWORD "123456"
#define WIFI_AP_SSID "Controller_AP"
#define WIFI_AP_PASSWORD "ap_password"

// Server definitions and declarations go here




#endif // SERVER_H  