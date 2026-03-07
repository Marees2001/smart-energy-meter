#ifndef SERVER_H
#define SERVER_H

#include <WiFi.h>
#include "controller.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

extern struct CONTROLLER_STATUS iemSta;
extern struct CONTROLLER_SETTINGS iemConf;
extern struct CONTROLLER_OPERATION iemOpt;
extern struct CONTROLLER_BILL iemBill[12];
extern char receive_buffer[200];
extern uint8_t network_status;
extern bool mqtt_received;
extern uint8_t publish_data;
extern ESP32Time rtc;

#define MQTT_BROKER "8c0734829583414dba307feccb737cf7.s1.eu.hivemq.cloud"
#define MQTT_PORT 8884
#define MQTT_TOPIC "iem/control"
#define MQTT_SUBSCRIBE "meter/command"

#define WIFI_STA_SSID "VARSHA F1 2G"
#define WIFI_STA_PASSWORD "123456"
#define WIFI_AP_SSID "my_iem"
#define WIFI_AP_PASSWORD "iem@123456"

#define IDLE 0
#define WIFI_CONNECT 1
#define MQTT_INIT 2
#define MQTT_CONNECT 3

#define STATUS_UPDATE 1
#define BILL_UPDATE 2

WiFiClient espClient;
PubSubClient mqttClient(espClient);
char* mqtt_client_id;
uint8_t bill_month = 0; // Variable to track the current billing month (0-11)

// Server definitions and declarations go here
void wifi_init(void) {
    Serial.println("Controller Starting...\r\n");
    WiFi.mode(WIFI_AP_STA);

    // Start Access Point
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    // Connect to WiFi
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
    WiFi.setAutoReconnect(true);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    // Handle incoming MQTT messages here
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        receive_buffer[i] = (char)payload[i];
    }
    receive_buffer[length] = '\0';
    Serial.println(receive_buffer);
    mqtt_received = true; // Set the flag to indicate a new MQTT message has been received
    // You can add code here to parse the payload and update controller settings
}

void mqtt_init() {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqtt_callback);
}

void mqttConnect() {
  Serial.println("Connecting MQTT...");
  sprintf(mqtt_client_id, "IEM_%s", WiFi.macAddress());
  Serial.println("MQTT client ID: ");
  Serial.println(mqtt_client_id);
  if (mqttClient.connect(mqtt_client_id)) {
    Serial.println("MQTT Connected");
    mqttClient.subscribe(MQTT_SUBSCRIBE);
    //mqttClient.publish(MQTT_TOPIC,"Device Online");
    network_status = MQTT_CONNECT;
    publish_data = STATUS_UPDATE; // Set to publish status update on next loop iteration
  }
}

void receive_data_from_mqtt() {
    if (mqtt_received) {
        // Parse the received MQTT message and update controller settings
        StaticJsonDocument<300> doc;
        DeserializationError error = deserializeJson(doc, receive_buffer);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }
        // Assuming the JSON has fields corresponding to CONTROLLER_SETTINGS
        if(doc.containsKey("method") && doc["method"] == "opt") {
            JsonObject params = doc["param"].as<JsonObject>();
            if(params.containsKey("s1")) iemOpt.rl1_opt = params["s1"];
            if(params.containsKey("s2")) iemOpt.rl2_opt = params["s2"];
            if(params.containsKey("s3")) iemOpt.rl3_opt = params["s3"];
        }
        else if(doc.containsKey("method") && doc["method"] == "getBill") {
            JsonObject params = doc["param"].as<JsonObject>();
            if(params.containsKey("month")) {
                bill_month = params["month"]; // Assuming month is 1-12 in the JSON, convert to 0-11 for array index
                bill_month = bill_month - 1; // Convert to 0-based index
                if(bill_month > 11) bill_month = 11; // Ensure month is within 0-11
            }
            publish_data = BILL_UPDATE;
        }
        doc.clear(); // Clear the document after processing
        mqtt_received = false; // Reset the flag after processing the message
    }
}

void publish_data_to_mqtt(uint8_t type) {
    // Create a JSON document to hold the data
    StaticJsonDocument<300> doc;
    switch (type) {
    case STATUS_UPDATE: {
        doc["method"] = "status";
        JsonObject param = doc.createNestedObject("param");
        param["sync"] = rtc.getEpoch(); // You might want to use millis() or a proper timestamp
        JsonObject dev = param.createNestedObject("dev");
        dev["nm"] = "My Energy Meter";
        dev["sta"] = iemSta.status ? 0 : 1; // Assuming 1 for active, 0 for offline, but JSON has 1
        dev["wifi"] = iemSta.wifi;
        dev["btt"] = iemSta.btt;
        JsonObject l1 = param.createNestedObject("l1");
        l1["sta"] = iemSta.rl1_status ? 1 : 0;
        l1["v"] = iemSta.ry_voltage;
        l1["A"] = iemSta.c1_current;
        JsonObject l2 = param.createNestedObject("l2");
        l2["sta"] = iemSta.rl2_status ? 1 : 0;
        l2["v"] = iemSta.yb_voltage;
        l2["A"] = iemSta.c2_current;
        JsonObject l3 = param.createNestedObject("l3");
        l3["sta"] = iemSta.rl3_status ? 1 : 0;
        l3["v"] = iemSta.br_voltage;
        l3["A"] = iemSta.c3_current;
        param["alm"] = iemSta.alm;  
        break;
    }
    case BILL_UPDATE: {
        doc["method"] = "billSta";
        JsonObject param = doc.createNestedObject("param");
        param["nm"] = iemBill[bill_month].nm;
        param["Cn"] = iemBill[bill_month].Cn;
        param["unit"] = iemBill[bill_month].unit;
        param["tariff"] = iemBill[bill_month].tariff;
        param["enrg"] = iemBill[bill_month].enrg;
        param["fix"] = iemBill[bill_month].fix;
        param["tax"] = iemBill[bill_month].tax;
        param["total"] = iemBill[bill_month].total;
        break;
    }
    default: {
        Serial.println("Unknown publish type");
        break;
    }
    }

    doc.shrinkToFit(); // Optimize memory usage
    char buffer[300];
    size_t n = serializeJson(doc, buffer);
    mqttClient.publish(MQTT_TOPIC, buffer, n);
    doc.clear(); // Clear the document after publishing
}

#endif // SERVER_H  