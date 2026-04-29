#ifndef SERVER_H
#define SERVER_H

#include <WiFi.h>
#include "controller.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <html.h>
#include <define.h>

extern struct CONTROLLER_STATUS iemSta;
extern struct CONTROLLER_SETTINGS iemConf;
extern struct CONTROLLER_OPERATION iemOpt;
extern struct CONTROLLER_BILL iemBill;
extern char receive_buffer[200];
extern uint8_t network_status;
extern bool mqtt_received;
extern uint8_t publish_data;
extern ESP32Time rtc;


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
    WiFi.begin(iemConf.net.ssid, iemConf.net.password);
    WiFi.setAutoReconnect(true);
}

void wifi_rssi() {
  long rssi = WiFi.RSSI();
  if (rssi >= -50) {
    iemSta.wifi = 3; // Excellent signal
  } else if (rssi >= -70) {
    iemSta.wifi = 2; // Good signal
  } else if (rssi >= -80) {
    iemSta.wifi = 1; // Weak signal
  } else {
    iemSta.wifi = 0; // No signal
  }
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
        param["nm"] = iemBill.nm;
        param["Cn"] = iemBill.Cn;
        param["unit"] = round(iemBill.CALC[bill_month].unit * 100.0) / 100.0;
        param["tariff"] = iemBill.tariff;
        param["enrg"] = iemBill.enrg;
        param["fix"] = iemBill.fix;
        param["tax"] = iemBill.tax;
        param["total"] = iemBill.CALC[bill_month].total;
        break;
    }
    default: {
        Serial.println("Unknown publish type");
        break;
    }
    }

    Serial.println("Publishing data to MQTT:[" + doc.as<String>() + "]");
    doc.shrinkToFit(); // Optimize memory usage
    char buffer[300];
    size_t n = serializeJson(doc, buffer);
    mqttClient.publish(MQTT_TOPIC,(const uint8_t*)buffer, n, false); // Publish with retain flag set to true
    doc.clear(); // Clear the document after publishing
}

void http_handle_settings_save(){
   String mode = server.arg("mode");

    // 🔹 Device Config
    if (mode == "1") {
        iemConf.volt.mn_ry_threshold = server.arg("vmin").toInt();
        iemConf.volt.mx_ry_threshold  = server.arg("vmax").toInt();
        float imin = server.arg("imin").toFloat();
        iemConf.curr.mx_c1_threshold = server.arg("imax").toFloat();

        Serial.println("Single Phase Config: MAX-"+String(iemConf.volt.mx_ry_threshold)
        +",MIN-"+String(iemConf.volt.mn_ry_threshold));
        Serial.println("Single Amp Config: MAX-"+String(iemConf.curr.mx_c1_threshold));
    }
    else {
        iemConf.volt.mx_ry_threshold = server.arg("vr_min").toInt();
        iemConf.volt.mn_ry_threshold = server.arg("vr_max").toInt();
        iemConf.curr.mx_c1_threshold = server.arg("iR_max").toFloat();

        iemConf.volt.mx_ry_threshold = server.arg("vy_min").toInt();
        iemConf.volt.mn_ry_threshold = server.arg("vy_max").toInt();
        iemConf.curr.mx_c2_threshold = server.arg("iY_max").toFloat();

        iemConf.volt.mx_yb_threshold = server.arg("vb_min").toInt();
        iemConf.volt.mn_yb_threshold = server.arg("vb_max").toInt();
        iemConf.curr.mx_c3_threshold = server.arg("iB_max").toFloat();
        Serial.println("3 Phase Config:");
        //Serial.println(vr_min);
    }

    // 🔹 Billing Config (COMMON)
    iemBill.nm    = server.arg("name");
    iemBill.Cn    = server.arg("consumer");
    iemBill.tariff = server.arg("tariff").toFloat();
    iemBill.enrg   = server.arg("energy").toFloat();
    iemBill.fix    = server.arg("fixed").toFloat();
    iemBill.tax    = server.arg("tax").toFloat();

    write_data_to_nvs("iem_conf", &iemConf, sizeof(iemConf)); // Save the default configuration to NVS
    write_data_to_nvs("iem_bill", &iemBill, sizeof(iemBill)); // Save the entire bill array to NVS

    // 🔹 Send response
    server.send(200, "text/html", "<h3>Saved Successfully</h3>");
}

void http_handle_settings(){
    Serial.println("Config page requested");
    server.send(200, "text/html", config_page);
}

void http_handle_wifi(){
    Serial.println("WiFi page requested");
    server.send(200, "text/html", wifi_page);
}

void handle_wifi_Save() {
    strcpy(iemConf.net.ssid, server.arg("ssid").c_str());
    strcpy(iemConf.net.password, server.arg("pass").c_str());
    Serial.println("SSID: " + String(iemConf.net.ssid));
    Serial.println("PASS: " + String(iemConf.net.password));
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
    server.on("/config", http_handle_settings);
    server.on("/config/save", http_handle_settings_save);
    server.begin();
    Serial.println("Server started");
}
#endif // SERVER_H  