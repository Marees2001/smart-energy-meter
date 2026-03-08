#include <controller.h>
#include <server.h>


struct CONTROLLER_STATUS iemSta;
struct CONTROLLER_SETTINGS iemConf;
struct CONTROLLER_OPERATION iemOpt;
struct CONTROLLER_BILL iemBill[12];

Preferences pref;
ESP32Time rtc; // Create an instance of the ESP32Time class to manage time functions
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS); // I2C LCD display at address 0x27, 16 columns, 4 rows

char receive_buffer[200]; // Buffer to hold incoming MQTT messages
bool mqtt_received = false; // Flag to indicate if a new MQTT message has been received
uint8_t network_status = IDLE;
uint8_t publish_data = STATUS_UPDATE; // 0 for status update, 1 for bill update

// Function declarations
void write_data_to_nvs(const char* key, const void* data, size_t len);
void read_data_from_nvs(const char* key, void* data, size_t len);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
    
  // Initialize controller bill with sample values
  //initialize the controller status, settings, and operation
  init_controller();
  // Initialize LCD display
  //init_lcd();
  // Initialize WiFi and MQTT
  wifi_init();
  // Setup HTTP server for WiFi configuration & Settings
  setup_server();
  
}

void loop() {
  static unsigned long network_timeout = 0;
  static unsigned long status_publish_timeout = 0;
  // Handle incoming HTTP requests
  server.handleClient();
  // put your main code here, to run repeatedly:
  if(millis() - network_timeout > 5000) { // Check every 5 seconds
    network_timeout = millis();
    if (WiFi.status() == WL_CONNECTED && network_status == IDLE) { 
      Serial.println("WiFi Connected");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      //NTP time synchronization can be added here if needed
      configTime(0, 0, "pool.ntp.org","time.nist.gov"); // Configure NTP with no timezone offset and no daylight saving time
      setenv("TZ", "IST-5:30:00", 1);
      tzset();
      struct tm timeinfo;
      if(getLocalTime(&timeinfo)) { // Synchronize time with NTP server
        rtc.setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
        timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      }
      Serial.print("Current Time: ");
      Serial.println(rtc.getTime("%Y-%m-%d %H:%M:%S"));
      network_status = WIFI_CONNECT;
    } else if(WiFi.status() == WL_CONNECTED && network_status == WIFI_CONNECT) {
      // WiFi is connected, you can add code here to handle MQTT or other tasks
      mqtt_init();
      Serial.println("MQTT Initialized");
      network_status = MQTT_INIT;
    } else if(WiFi.status() == WL_CONNECTED && network_status == MQTT_INIT) {
      // WiFi is still connected, you can add code here to handle MQTT or other tasks
      mqttConnect();
    }else if(!mqttClient.connected() && network_status == MQTT_CONNECT) {
      Serial.println("MQTT Disconnected");
      network_status = MQTT_INIT;
    }else if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Disconnected");
      network_status = IDLE;
    }
  }

  wifi_rssi(); // Update WiFi signal strength in controller status
  btt_status(); // Update battery status in controller status

  if(millis()-status_publish_timeout > 15000) { // Publish status update every 15 seconds
    status_publish_timeout = millis();
    publish_data = STATUS_UPDATE;
  }
  
  if(network_status == MQTT_CONNECT) {
    mqttClient.loop();
    receive_data_from_mqtt();
    if(publish_data) {
      publish_data_to_mqtt(publish_data);
      publish_data = 0;
    }
  }
}


void write_data_to_nvs(const char* key, const void* data, size_t len) {
  pref.begin("iem_data", false); // Open the namespace "iem_data" in RW mode
  pref.putBytes(key, data, len); // Write the data to NVS
  pref.end(); // Close the Preferences
}

void read_data_from_nvs(const char* key, void* data, size_t len) {
  pref.begin("iem_data", false); // Open the namespace "iem_data" in RW mode
  pref.getBytes(key, data, len); // Read the data from NVS
  pref.end(); // Close the Preferences
}