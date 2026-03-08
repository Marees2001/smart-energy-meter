#include <controller.h>
#include <server.h>


struct CONTROLLER_STATUS iemSta;
struct CONTROLLER_SETTINGS iemConf;
struct CONTROLLER_OPERATION iemOpt;
struct CONTROLLER_BILL iemBill[12];
struct POWER_CALCULATION iemPwr;

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
  static unsigned long voltage_read_timeout = 0;
  static unsigned long current_read_timeout = 0;
  static unsigned long power_calc_timeout = 0;

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

  /*Function to read voltages*/
  if(millis()-voltage_read_timeout > 10000) { // Read voltages every 10 seconds
    voltage_read_timeout = millis();
    adc_read_voltages();
  }

  /*Function to read currents*/
  if(millis()-current_read_timeout > 1000) { // Read currents every 1 second
    current_read_timeout = millis();
    adc_read_currents();
  }

  /*Function to calculate power*/
  if(millis()-power_calc_timeout > 5000) { // Calculate power every 5 seconds
    power_calc_timeout = millis();
    iemPwr.bill_month = rtc.getMonth(); // Update billing month based on current time
    static unsigned long last_ry_power_calc_time = 0;
    static unsigned long last_yb_power_calc_time = 0;
    static unsigned long last_br_power_calc_time = 0;
    float tmp_rypower = iemSta.ry_voltage * iemSta.c1_current * POWER_FACTOR; // P = V * I * PF 
    float tmp_ybpower = iemSta.yb_voltage * iemSta.c2_current * POWER_FACTOR; // P = V * I * PF
    float tmp_brpower = iemSta.br_voltage * iemSta.c3_current * POWER_FACTOR; // P = V * I * PF
    // Update power values only if they have changed significantly to avoid noise
    if (abs(tmp_rypower - iemPwr.ry_power) > 1.0) { // Update if change is greater than 1 watt
      iemPwr.ry_power = tmp_rypower;
      iemPwr.ry_load_hours += (millis() - last_ry_power_calc_time) / 3600000.0; // Increment load hours based on time elapsed
      last_ry_power_calc_time = millis();
    }
    if (abs(tmp_ybpower - iemPwr.yb_power) > 1.0) { // Update if change is greater than 1 watt
      iemPwr.yb_power = tmp_ybpower;
      iemPwr.yb_load_hours += (millis() - last_yb_power_calc_time) / 3600000.0; // Increment load hours based on time elapsed
      last_yb_power_calc_time = millis();
    }
    if (abs(tmp_brpower - iemPwr.br_power) > 1.0) { // Update if change is greater than 1 watt
      iemPwr.br_power = tmp_brpower;
      iemPwr.br_load_hours += (millis() - last_br_power_calc_time) / 3600000.0; // Increment load hours based on time elapsed
      last_br_power_calc_time = millis();
    }

    iemBill[iemPwr.bill_month].enrg = (iemPwr.ry_power * iemPwr.ry_load_hours) 
                                    + (iemPwr.yb_power * iemPwr.yb_load_hours) 
                                    + (iemPwr.br_power * iemPwr.br_load_hours); // Energy in watt-hours
    iemBill[iemPwr.bill_month].enrg = iemBill[iemPwr.bill_month].enrg / 1000.0; // Convert to kWh
    if(iemPwr.bill_type != 1 || iemBill[iemPwr.bill_month].enrg > 100)
    iemBill[iemPwr.bill_month].total = iemBill[iemPwr.bill_month].fix 
        + (iemBill[iemPwr.bill_month].tariff * iemBill[iemPwr.bill_month].enrg / 1000.0) 
        + iemBill[iemPwr.bill_month].tax; // Total bill amount
  }

  /*Function to publish status updates*/
  if(millis()-status_publish_timeout > 30000L) { // Publish status update every 30 seconds
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