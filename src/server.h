#ifndef SERVER_H
#define SERVER_H

#include <WiFi.h>
#include "controller.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <html.h>

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
#define MQTT_PORT 8883
#define MQTT_USERNAME "Marees"
#define MQTT_PASSWORD "Marees2001"
#define MQTT_TOPIC "iem/control"
#define MQTT_SUBSCRIBE "meter/command"

#define WIFI_STA_SSID "Alpha_geek"
#define WIFI_STA_PASSWORD "Alpha_geek2001"
#define WIFI_AP_SSID "my_iem"
#define WIFI_AP_PASSWORD "iem@123456"

#define IDLE 0
#define WIFI_CONNECT 1
#define MQTT_INIT 2
#define MQTT_CONNECT 3

#define STATUS_UPDATE 1
#define BILL_UPDATE 2

WiFiClientSecure espClient;
WebServer server(80);
PubSubClient mqttClient(espClient);
char mqtt_client_id[40];
uint8_t bill_month = 0; // Variable to track the current billing month (0-11)

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygch
77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+0T
K+fLrR2WSUcLkcwHjBWovW5L5lyXcU1c5KRH+uGJjGzE4O7Ee7zLJXcQXSMkdQi8
7Oq4rQGTTJvO7R96nvPB3vw2YBVHfPfWK9JCKnTfOQGzYv2P5V+BPXTyKJhGdWD1
VO4b3ihoKdJVdZ4jZAc8HJUiVY8arjFnAHJibGrM+6/xJV2V4nbtXmW8V6GkwHnP
nXVTK8C6cPJRG9q4y2j8ZKGKb3xCPTH3HCe2LuG3bjI4rZWHJhJFKG7fBjOxsdxg
dWGKdD1P9aNyBWgv0U0zDkR1V7wqb1pLZPj4M4sTJ5qX9vfIYGnk4rQJLp4QEo0
wWLiu1u1pfRy3UpjPJzKlWnvY8AwY3xYqPjWp8OIggDxAdR8ob1U+Lq0cDc7fsI
8q5X+4L8N+POCzJGt4l8P+6XMGWZA3YpilV8CzNkJBAj+bwqLQb8c5UOqMB3HL5
VjCnTJCuEPoQBcSyjq6jVqX6B1LwPBZ6GLXL1H5zJ8i0Qm7PgI3fkiEkEOt1l6Q
0Qd5kYFp0XC1R1tWLWG+kHlB3TKr+XcEoI4jUq7Kg9gTJxV7HNRUoqK6zE7j8h4
K7B3c9QECBGxWXB9KvqTzK2ZG6QB8NTGjEi
-----END CERTIFICATE-----
)EOF";


// Server definitions and declarations go here
void wifi_init(void) {
    Serial.println("Controller wifi Starting...\r\n");
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
    espClient.setInsecure();   // skip certificate validation (temporary for testing)
    // espClient.setCACert(root_ca);
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqtt_callback);
}

void mqttConnect() {
  Serial.println("Connecting MQTT...");
  sprintf(mqtt_client_id, "IEM_%s", WiFi.macAddress().c_str());
  Serial.println("MQTT client ID: ");
  Serial.println(mqtt_client_id);
  if (mqttClient.connect(mqtt_client_id,MQTT_USERNAME,MQTT_PASSWORD)) {
    Serial.println("MQTT Connected");
    mqttClient.subscribe(MQTT_SUBSCRIBE);
    //mqttClient.publish(MQTT_TOPIC,"Device Online");
    network_status = MQTT_CONNECT;
    publish_data = STATUS_UPDATE; // Set to publish status update on next loop iteration
  }else {
    Serial.print("MQTT failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void receive_data_from_mqtt() {
    if (mqtt_received) {
        // Parse the received MQTT message and update controller settings
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, receive_buffer);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }
        // Assuming the JSON has fields corresponding to CONTROLLER_SETTINGS
        if (!doc["method"].isNull() && doc["method"] == "opt") {
            JsonObject params = doc["param"];
            if (!params["s1"].isNull()) iemOpt.rl1_opt = params["s1"];
            if (!params["s2"].isNull()) iemOpt.rl2_opt = params["s2"];
            if (!params["s3"].isNull()) iemOpt.rl3_opt = params["s3"];
        }
        else if (!doc["method"].isNull() && doc["method"] == "getBill") {
            JsonObject params = doc["param"];
            if (!params["month"].isNull()) {
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
    JsonDocument doc;
    switch (type) {
    case STATUS_UPDATE: {
        doc["method"] = "status";
        JsonObject param = doc["param"].to<JsonObject>();
        param["sync"] = rtc.getEpoch(); // You might want to use millis() or a proper timestamp
        JsonObject dev = param["dev"].to<JsonObject>();
        dev["nm"] = "My Energy Meter";
        dev["sta"] = iemSta.status ? 0 : 1; // Assuming 1 for active, 0 for offline, but JSON has 1
        dev["wifi"] = iemSta.wifi;
        dev["btt"] = iemSta.btt;
        JsonObject l1 = param["l1"].to<JsonObject>();
        l1["sta"] = iemSta.rl1_status ? 1 : 0;
        l1["v"] = iemSta.ry_voltage;
        l1["A"] = iemSta.c1_current;
        JsonObject l2 = param["l2"].to<JsonObject>();
        l2["sta"] = iemSta.rl2_status ? 1 : 0;
        l2["v"] = iemSta.yb_voltage;
        l2["A"] = iemSta.c2_current;
        JsonObject l3 = param["l3"].to<JsonObject>();
        l3["sta"] = iemSta.rl3_status ? 1 : 0;
        l3["v"] = iemSta.br_voltage;
        l3["A"] = iemSta.c3_current;
        param["alm"] = iemSta.alm;  
        break;
    }
    case BILL_UPDATE: {
        doc["method"] = "billSta";
        JsonObject param = doc["param"].to<JsonObject>();
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


void http_handle_wifi(){
    Serial.println("WiFi page requested");
    server.send(200, "text/html", wifi_page);
}

void handle_wifi_Save() {
  iemConf.net.ssid = server.arg("ssid");
  iemConf.net.password = server.arg("pass");
  Serial.println("SSID: " + iemConf.net.ssid);
  Serial.println("PASS: " + iemConf.net.password);
  server.send(200,"text/html","<h3>Connecting to WiFi...</h3>");
  //WiFi.begin(iemConf.net.ssid.c_str(), iemConf.net.password.c_str());
  write_data_to_nvs("iem_conf", &iemConf, sizeof(iemConf)); // Save the configuration to NVS
  delay(2000);
  ESP.restart(); // Restart the ESP to apply the new configuration
}

void setup_server() {
  Serial.println("Setting up server...");
  server.on("/", [](){ server.send(200, "text/plain", "ESP32 Server Running"); });
  server.on("/wifi", http_handle_wifi);
  server.on("/wifi/save", handle_wifi_Save);
  server.begin();
  Serial.println("Server started");
}
#endif // SERVER_H  