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
void write_float_to_nvs(const char* key, float value);
float read_float_from_nvs(const char* key);

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
  static unsigned long bill_save_timeout = 0;

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
      configTzTime("IST-5:30", "pool.ntp.org","time.nist.gov"); // Configure NTP with no timezone offset and no daylight saving time
      struct tm timeinfo;
      static unsigned long ntp_sync_timeout = millis();
      while(!getLocalTime(&timeinfo) && millis() - ntp_sync_timeout < 10000) { // Synchronize time with NTP server
        Serial.println("Waiting for NTP time sync...");
        delay(1000);
      }
      if(getLocalTime(&timeinfo)) rtc.setTimeStruct(timeinfo); // Set RTC time from NTP
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

  /*Function to read voltages*/
  if(millis()-voltage_read_timeout > 10000) { // Read voltages every 10 seconds
    voltage_read_timeout = millis();
    adc_read_voltages();
    wifi_rssi(); // Update WiFi signal strength in controller status
    btt_status(); // Update battery status in controller status
  }

  /*Function to read currents*/
  if(millis()-current_read_timeout > 1000) { // Read currents every 1 second
    current_read_timeout = millis();
    adc_read_currents();
  }

  /*function for control relay1 from manual input*/
  if(iemOpt.rl1_opt != iemSta.rl1_status) {
    iemSta.rl1_status = iemOpt.rl1_opt;
    digitalWrite(RELAY1_PIN, iemOpt.rl1_opt ? HIGH : LOW);
    publish_data = STATUS_UPDATE; // Trigger status update after relay state change
  }
  /*function for control relay2 from manual input*/
  if(iemOpt.rl2_opt != iemSta.rl2_status) {
    iemSta.rl2_status = iemOpt.rl2_opt;
    digitalWrite(RELAY2_PIN, iemOpt.rl2_opt ? HIGH : LOW);
    publish_data = STATUS_UPDATE; // Trigger status update after relay state change
  }
  /*function for control relay3 from manual input*/
  if(iemOpt.rl3_opt != iemSta.rl3_status) {
    iemSta.rl3_status = iemOpt.rl3_opt;
    digitalWrite(RELAY3_PIN, iemOpt.rl3_opt ? HIGH : LOW);
    publish_data = STATUS_UPDATE; // Trigger status update after relay state change
  }

  /*Function to check alarms*/
  check_alarms();

  /* ---------------------------------------------------------------
   * Power calculation section
   * This block is executed every POWER_CALCULATION_INTERVAL milliseconds
   * (typically 5000 ms or 5 seconds) and performs the following:
   * 1. Determine the current billing month from the RTC.
   * 2. Compute instantaneous real power for each phase (RY, YB, BR) using
   *    the formula: P = V * I * PF, where PF is the power factor constant.
   * 3. Sum the phase powers to get total_power.
   * 4. Calculate the energy increment for this interval:
   *    energy_increment = total_power * (POWER_CALCULATION_INTERVAL / 1000) / 3600
   *    This gives energy in watt-hours (Wh) for the interval.
   * 5. Convert energy_increment to kilowatt-hours (kWh) by dividing by 1000.
   * 6. Accumulate the energy_increment into the monthly energy counter
   *    iemBill[bill_month].enrg.
   * 7. Compute the total bill amount if bill_type != 1 or if energy > 100 kWh:
   *    total = fixed_charge + (tariff_rate * accumulated_energy_kWh) + tax
   *
   * Variables involved (defined in controller.h):
   *   - iemSta: Contains voltage and current measurements.
   *   - iemPwr: Holds power values and billing info like bill_month and bill_type.
   *   - iemBill[]: Array for monthly bill data including energy (enrg) and total.
   *
   * Notes:
   * - POWER_FACTOR and POWER_CALCULATION_INTERVAL are constants defined elsewhere.
   * - This approach assumes constant power over the interval for energy calculation.
   * - Adjusting POWER_FACTOR or the interval affects measurement accuracy.
   * --------------------------------------------------------------- */
  if(millis()-power_calc_timeout > POWER_CALCULATION_INTERVAL) { // Calculate power every 5 seconds
    power_calc_timeout = millis();
    iemPwr.bill_month = rtc.getMonth(); // Update billing month based on current time

    iemPwr.ry_power = iemSta.ry_voltage * iemSta.c1_current * POWER_FACTOR; // P = V * I * PF 
    iemPwr.yb_power = iemSta.yb_voltage * iemSta.c2_current * POWER_FACTOR; // P = V * I * PF
    iemPwr.br_power = iemSta.br_voltage * iemSta.c3_current * POWER_FACTOR; // P = V * I * PF
    // Total power
    float total_power = iemPwr.ry_power + iemPwr.yb_power + iemPwr.br_power;
    // Energy accumulation (Wh)
    float energy_increment = (total_power * POWER_CALCULATION_INTERVAL) / 3600000.0;
    // Convert to kWh
    energy_increment = energy_increment / 1000.0;
    iemBill[iemPwr.bill_month].enrg += energy_increment; // Accumulate energy for the month
    // Bill calculated for all bill types, but for bill_type is home only if energy exceeds 100 kWh
    if(iemPwr.bill_type != 1 || iemBill[iemPwr.bill_month].enrg > 100)
    iemBill[iemPwr.bill_month].total = iemBill[iemPwr.bill_month].fix 
        + (iemBill[iemPwr.bill_month].tariff * iemBill[iemPwr.bill_month].enrg) 
        + iemBill[iemPwr.bill_month].tax; // Total bill amount
  }

  /*Function to save bill data*/
  if(millis()-bill_save_timeout > 60000L) { // Save bill data every 60 seconds
    bill_save_timeout = millis();
    write_data_to_nvs("iem_bill", iemBill, sizeof(iemBill)); // Save the entire bill array to NVS
    write_data_to_nvs("iem_pwr", &iemPwr, sizeof(iemPwr)); // Save power calculation data to NVS
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

void write_float_to_nvs(const char* key, float value) {
  pref.begin("iem_data", false); // Open the namespace "iem_data" in RW mode
  pref.putFloat(key, value); // Write the float value to NVS
  pref.end(); // Close the Preferences
}

void read_float_from_nvs(const char* key, float* value) {
  pref.begin("iem_data", false); // Open the namespace "iem_data" in RW mode
  *value = pref.getFloat(key, 0.0); // Read the float value from NVS, default to 0.0 if not found
  pref.end(); // Close the Preferences
}