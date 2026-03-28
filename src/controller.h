
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <time.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ZMPT101B.h"
#include <define.h>

// Forward declaration of LCD object
extern LiquidCrystal_I2C lcd;
ZMPT101B ry_voltSens(PHASE1_ADC_PIN);
ZMPT101B yb_voltSens(PHASE2_ADC_PIN);
ZMPT101B br_voltSens(PHASE3_ADC_PIN);

extern struct CONTROLLER_STATUS iemSta;
extern struct CONTROLLER_SETTINGS iemConf;
extern struct CONTROLLER_OPERATION iemOpt;
extern struct CONTROLLER_BILL iemBill[12];
extern struct POWER_CALCULATION iemPwr;

extern uint8_t publish_data;

extern void write_data_to_nvs(const char* key, const void* data, size_t len);
extern void read_data_from_nvs(const char* key, void* data, size_t len);

/*Structure definition for Controller status*/
struct CONTROLLER_STATUS {
  // Add any necessary member variables here
  bool status; // 0 for Active, 1 for Offline
  uint8_t btt; // range fro 0 to 3. 0-> 0%, 1-> 25%, 2-> 50%, 3-> 100%
  uint8_t wifi; // range from 0 to 3. 0-> 0%, 1-> 25%, 2-> 50%, 3-> 100%
  uint8_t alm; // range from 0 to 5. 0-> No Alarm, 1-> Power shutdown, 2-> Overload, 3-> Overvoltage, 4-> Undervoltage, 5-> Short Circuit
  int16_t ry_voltage; // range from 0 to 500V
  int16_t yb_voltage; // range from 0 to 500V
  int16_t br_voltage; // range from 0 to 500V
  float c1_current; // range from 0 to 100A
  float c2_current; // range from 0 to 100A
  float c3_current; // range from 0 to 100A

  bool rl1_status; // 0 for OFF, 1 for ON
  bool rl2_status; // 0 for OFF, 1 for ON 
  bool rl3_status; // 0 for OFF, 1 for ON
};

/*Structure definition for voltage settings*/
struct VOLTAGE_SETTINGS {
  // Add any necessary member variables here
  int16_t mx_ry_threshold; // range from 0 to 500V
  int16_t mx_yb_threshold; // range from 0 to 500V
  int16_t mx_br_threshold; // range from 0 to 500V
  int16_t mn_ry_threshold; // range from 0 to 500V
  int16_t mn_yb_threshold; // range from 0 to 500V  
  int16_t mn_br_threshold; // range from 0 to 500V
};

/*Structure definition for current settings*/
struct CURRENT_SETTINGS {
  // Add any necessary member variables here
  float mx_c1_threshold; // range from 0 to 100A
  float mx_c2_threshold; // range from 0 to 100A
  float mx_c3_threshold; // range from 0 to 100A
};

/*Structure definition for network settings*/
struct NETWORK_SETTINGS {
  // Add any necessary member variables here
  char ssid[32]; // WiFi SSID (max 31 characters + null terminator)
  char password[64]; // WiFi Password (max 63 characters + null terminator
  String ip_addr;
};

/*Structure definition for Controller settings*/
struct CONTROLLER_SETTINGS {
  // Add any necessary member variables here
  float version; // Firmware version
  VOLTAGE_SETTINGS volt;
  CURRENT_SETTINGS curr;
  NETWORK_SETTINGS net;
};

/*Structure definition for Controller operation*/
struct CONTROLLER_OPERATION {
  // Add any necessary member variables here
  bool rl1_opt = false; // 0 for OFF, 1 for ON
  bool rl2_opt = false; // 0 for OFF, 1 for ON 
  bool rl3_opt = false; // 0 for OFF, 1 for ON
};

/*Structure definition for Controller bill*/
struct CONTROLLER_BILL {
  String nm; // Name
  String Cn; // Consumer number
  float unit; // Units consumed
  uint8_t tariff; // Tariff rate
  float enrg; // Energy charge
  float fix; // Fixed charge
  float tax; // Tax
  float total; // Total amount
};

/*Structure definition for power calculation*/
struct POWER_CALCULATION {
  float ry_power; // Power for RY phase
  float yb_power; // Power for YB phase
  float br_power; // Power for BR phase
  long ry_load_hours; // Load hours for RY phase
  long yb_load_hours; // Load hours for YB phase
  long br_load_hours; // Load hours for BR phase
  uint8_t bill_month; // Month for billing
  uint8_t bill_type; // Segment for billing (e.g., 1->home, 2->commercial, 3->industrial)
};

void init_controller() {
  Serial.println("Controller init...\r\n");
  // Initialize controller status
  read_data_from_nvs("iem_conf", &iemConf, sizeof(iemConf));
  if(iemConf.version != FIRMWARE_VERSION) { // Check if the version matches expected value
    Serial.println("No valid configuration found in NVS, initializing with default values.");
    iemConf.version = FIRMWARE_VERSION;
    iemSta.status = 0; // Active
    iemSta.btt = 0; // 0% battery
    iemSta.wifi = 0; // 0% WiFi signal
    iemSta.alm = 0; // No Alarm
    iemSta.ry_voltage = 0;
    iemSta.yb_voltage = 0;
    iemSta.br_voltage = 0;
    iemSta.c1_current = 0.0;
    iemSta.c2_current = 0.0;
    iemSta.c3_current = 0.0;
    iemSta.rl1_status = false; // OFF
    iemSta.rl2_status = false; // OFF
    iemSta.rl3_status = false; // OFF

    // Initialize controller settings
    iemConf.version = FIRMWARE_VERSION; // Set version to indicate valid configuration
    iemConf.volt.mx_ry_threshold = 240; // Example threshold values
    iemConf.volt.mx_yb_threshold = 240;
    iemConf.volt.mx_br_threshold = 240;
    iemConf.volt.mn_ry_threshold = 180;
    iemConf.volt.mn_yb_threshold = 180;
    iemConf.volt.mn_br_threshold = 180;
    iemConf.curr.mx_c1_threshold = 5.0; // Example threshold values
    iemConf.curr.mx_c2_threshold = 5.0;
    iemConf.curr.mx_c3_threshold = 5.0;
    strcpy(iemConf.net.ssid, DEFAULT_WIFI_STA_SSID);
    strcpy(iemConf.net.password, DEFAULT_WIFI_STA_PASSWORD);
    
    // Initialize controller operation
    iemOpt.rl1_opt = false; // OFF
    iemOpt.rl2_opt = false; // OFF
    iemOpt.rl3_opt = false; // OFF

    // Initialize controller bill
    for(int i = 0; i < 12; i++) {
      iemBill[i].nm = "";
      iemBill[i].Cn = "";
      iemBill[i].unit = 0;
      iemBill[i].tariff = 0;
      iemBill[i].enrg = 0;
      iemBill[i].fix = 0;
      iemBill[i].tax = 0;
      iemBill[i].total = 0;
    }
    write_data_to_nvs("iem_conf", &iemConf, sizeof(iemConf)); // Save the default configuration to NVS
    write_data_to_nvs("iem_bill", iemBill, sizeof(iemBill)); // Save the default bill data to NVS
    write_data_to_nvs("iem_pwr", &iemPwr, sizeof(iemPwr)); // Save the default power calculation data to NVS
  } else {
    Serial.println("Valid configuration found in NVS, loading...");
    read_data_from_nvs("iem_conf", &iemConf, sizeof(iemConf));
    read_data_from_nvs("iem_bill", iemBill, sizeof(iemBill));
    read_data_from_nvs("iem_pwr", &iemPwr, sizeof(iemPwr));
  }
  //Initialize the GPIO pins
  // Digital Output Pins (Relays)
  pinMode(RELAY1_PIN, OUTPUT); // RELAY_Y (Line 1) - GPIO38
  pinMode(RELAY2_PIN, OUTPUT); // RELAY_Y (Line 2) - GPIO45
  pinMode(RELAY3_PIN, OUTPUT); // RELAY_B (Line 3) - GPIO46
  digitalWrite(RELAY1_PIN, LOW);  // Turn off relay 1
  digitalWrite(RELAY2_PIN, LOW);  // Turn off relay 2
  digitalWrite(RELAY3_PIN, LOW);  // Turn off relay 3
  
  // Digital Input Pins
  pinMode(BUTTON_PIN, INPUT); // BUTTON - GPIO37

  // Initialize I2C for LCD display
  Wire.begin(MCU_SDA_PIN, MCU_SCL_PIN);
  //setup button interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), [](){
    iemOpt.rl1_opt = LOW; // Toggle relay 1 operation on button press
    iemOpt.rl2_opt = LOW; // Toggle relay 2 operation on button press
    iemOpt.rl3_opt = LOW; // Toggle relay 3 operation on button press
  }, FALLING);
}

void init_lcd() {
  // Initialize I2C LCD display
  lcd.init();
  lcd.backlight();
  
  // Display welcome message
  lcd.setCursor(0, 0);
  lcd.print("Energy Meter");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  delay(2000);
  lcd.clear();
}

void btt_status() {
  // Read battery voltage from ADC pin
  // ESP32 ADC: 12-bit (0-4095), max 3.3V
  int adc_value = analogRead(BATT_ADC_PIN);
  // Convert ADC reading to voltage in millivolts
  // Assuming voltage divider is configured to read up to 4.2V
  float voltage_mv = (adc_value / 4095.0) * 3300.0 * 1.27; // 1.27 factor for voltage divider correction
  // Map voltage range 3000mV (0%) to 4200mV (100%) to level 0-3
  if (voltage_mv <= 3000) {
    iemSta.btt = 0; // 0%
  } else if (voltage_mv <= 3400) {
    iemSta.btt = 1; // ~25%
  } else if (voltage_mv <= 3800) {
    iemSta.btt = 2; // ~50%
  } else {
    iemSta.btt = 3; // 100%
  }
}

void adc_read_voltages() {
  // Read voltages from ADC pins and update iemSta
  iemSta.ry_voltage = ry_voltSens.getRmsVoltage(10); // Average of 10 readings for better accuracy
  iemSta.yb_voltage = yb_voltSens.getRmsVoltage(10); // Average of 10 readings for better accuracy
  iemSta.br_voltage = br_voltSens.getRmsVoltage(10); // Average of 10 readings for better accuracy

  Serial.println("RY Voltage: "+String(iemSta.ry_voltage)+" V");
  Serial.println("YB Voltage: "+String(iemSta.yb_voltage)+" V");
  Serial.println("BR Voltage: "+String(iemSta.br_voltage)+" V");

  // Check RY voltage against thresholds and update alarm status
  if (iemSta.ry_voltage > iemConf.volt.mx_ry_threshold) {
    iemSta.alm = 3; // Overvoltage
    digitalWrite(RELAY1_PIN, HIGH); // Turn ON relay 1 to move NC to NO and disconnect load
    iemSta.rl1_status = true; // Update relay status in controller status
    iemOpt.rl1_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  } else if (iemSta.ry_voltage < iemConf.volt.mn_ry_threshold) {
    iemSta.alm = 4; // Undervoltage
    digitalWrite(RELAY1_PIN, HIGH); // Turn ON relay 1 to move NC to NO and disconnect load
    iemSta.rl1_status = true; // Update relay status in controller status
    iemOpt.rl1_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  }

  // Check YB voltage against thresholds and update alarm status
  if (iemSta.yb_voltage > iemConf.volt.mx_yb_threshold) {
    iemSta.alm = 3; // Overvoltage
    digitalWrite(RELAY2_PIN, HIGH); // Turn ON relay 2 to move NC to NO and disconnect load
    iemSta.rl2_status = true; // Update relay status in controller status
    iemOpt.rl2_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  } else if (iemSta.yb_voltage < iemConf.volt.mn_yb_threshold) {
    iemSta.alm = 4; // Undervoltage
    digitalWrite(RELAY2_PIN, HIGH); // Turn ON relay 2 to move NC to NO and disconnect load
    iemSta.rl2_status = true; // Update relay status in controller status
    iemOpt.rl2_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  }

  // Check BR voltage against thresholds and update alarm status
  if (iemSta.br_voltage > iemConf.volt.mx_br_threshold) {
    iemSta.alm = 3; // Overvoltage
    digitalWrite(RELAY3_PIN, HIGH); // Turn ON relay 3 to move NC to NO and disconnect load
    iemSta.rl3_status = true; // Update relay status in controller status
    iemOpt.rl3_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  } else if (iemSta.br_voltage < iemConf.volt.mn_br_threshold) {
    iemSta.alm = 4; // Undervoltage
    digitalWrite(RELAY3_PIN, HIGH); // Turn ON relay 3 to move NC to NO and disconnect load
    iemSta.rl3_status = true; // Update relay status in controller status
    iemOpt.rl3_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  }
}

float get_current(uint8_t adc_pin) {
    float sum = 0.0;
    float sum_raw = 0.0;
    float samples[500];

    // Step 1: Collect samples with timing
    for (int i = 0; i < 500; i++) {
        samples[i] = (analogRead(adc_pin) / 4095.0) * 3.3;
        sum_raw += samples[i];
        delayMicroseconds(100);
    }

    // Step 2: Compute dynamic DC offset
    float mean = sum_raw / 500.0;

    // Step 3: Compute RMS of centered signal
    for (int i = 0; i < 500; i++) {
        float centered = samples[i] - mean;
        sum += centered * centered;
    }

    float rmsVoltage = sqrt(sum / 500.0f);

    // CALIBRATION_FACTOR = turns_ratio / burden_resistor (e.g., 1000 / 150.0)
    float current = rmsVoltage * CALIBRATION_FACTOR;
    return current;
}

void adc_read_currents() {
  // Read currents from ADC pins and update iemSta
  iemSta.c1_current = get_current(CURRENT1_ADC_PIN);
  iemSta.c2_current = get_current(CURRENT2_ADC_PIN);
  iemSta.c3_current = get_current(CURRENT3_ADC_PIN);

  Serial.println("C1 Current: "+String(iemSta.c1_current)+" A");
  Serial.println("C2 Current: "+String(iemSta.c2_current)+" A");
  Serial.println("C3 Current: "+String(iemSta.c3_current)+" A");

  // Check CT1 current against threshold and update alarm status
  if (iemSta.c1_current > iemConf.curr.mx_c1_threshold) {
    iemSta.alm = 2; // Overload
    digitalWrite(RELAY1_PIN, HIGH); // Turn ON relay 1 to move NC to NO and disconnect load
    iemSta.rl1_status = true; // Update relay status in controller status
    iemOpt.rl1_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  }

  // Check CT2 current against threshold and update alarm status
  if (iemSta.c2_current > iemConf.curr.mx_c2_threshold) {
    iemSta.alm = 2; // Overload
    digitalWrite(RELAY2_PIN, HIGH); // Turn ON relay 2 to move NC to NO and disconnect load
    iemSta.rl2_status = true; // Update relay status in controller status
    iemOpt.rl2_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  } 

  // Check CT3 current against threshold and update alarm status
  if (iemSta.c3_current > iemConf.curr.mx_c3_threshold) {
    iemSta.alm = 2; // Overload
    digitalWrite(RELAY3_PIN, HIGH); // Turn ON relay 3 to move NC to NO and disconnect load
    iemSta.rl3_status = true; // Update relay status in controller status
    iemOpt.rl3_opt = true; // Update relay operation in controller operation
    publish_data = STATUS_UPDATE; // Trigger MQTT publish for status update
  }
}

#endif // CONTROLLER_H