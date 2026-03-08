
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <time.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ZMPT101B.h"

#define MCU_SDA_PIN 21
#define MCU_SCL_PIN 22
#define BATT_ADC_PIN 34
#define PHASE1_ADC_PIN 39
#define PHASE2_ADC_PIN 36
#define PHASE3_ADC_PIN 35
#define CURRENT1_ADC_PIN 32
#define CURRENT2_ADC_PIN 33
#define CURRENT3_ADC_PIN 25
#define RELAY1_PIN 38
#define RELAY2_PIN 45
#define RELAY3_PIN 46
#define BUTTON_PIN 37

#define LCD_ADDR 0x27  // I2C address for 16x4 LCD display
#define LCD_COLS 16    // LCD display columns
#define LCD_ROWS 4     // LCD display rows

#define POWER_FACTOR 0.8 // Power factor for energy calculation
#define CALIBRATION_FACTOR 30.0 // Example calibration factor for current sensor

// Forward declaration of LCD object
extern LiquidCrystal_I2C lcd;
ZMPT101B ry_voltSens(PHASE1_ADC_PIN);
ZMPT101B yb_voltSens(PHASE2_ADC_PIN);
ZMPT101B br_voltSens(PHASE3_ADC_PIN);

extern struct CONTROLLER_STATUS iemSta;
extern struct CONTROLLER_SETTINGS iemConf;
extern struct CONTROLLER_OPERATION iemOpt;
extern struct CONTROLLER_BILL iemBill[12];

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
  String ssid;
  String password;
  String ip_addr;

};

/*Structure definition for Controller settings*/
struct CONTROLLER_SETTINGS {
  // Add any necessary member variables here
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
  // iemSta.status = 0; // Active
  // iemSta.btt = 0; // 0% battery
  // iemSta.wifi = 0; // 0% WiFi signal
  // iemSta.alm = 0; // No Alarm
  // iemSta.ry_voltage = 0;
  // iemSta.yb_voltage = 0;
  // iemSta.br_voltage = 0;
  // iemSta.c1_current = 0.0;
  // iemSta.c2_current = 0.0;
  // iemSta.c3_current = 0.0;
  // iemSta.rl1_status = false; // OFF
  // iemSta.rl2_status = false; // OFF
  // iemSta.rl3_status = false; // OFF

  // // Initialize controller settings
  // iemConf.volt.mx_ry_threshold = 240; // Example threshold values
  // iemConf.volt.mx_yb_threshold = 240;
  // iemConf.volt.mx_br_threshold = 240;
  // iemConf.volt.mn_ry_threshold = 180;
  // iemConf.volt.mn_yb_threshold = 180;
  // iemConf.volt.mn_br_threshold = 180;
  // iemConf.curr.mx_c1_threshold = 10.0; // Example threshold values
  // iemConf.curr.mx_c2_threshold = 10.0;
  // iemConf.curr.mx_c3_threshold = 10.0;
  // iemConf.net.ip_addr = "";
  
  // // Initialize controller operation
  // iemOpt.rl1_opt = false; // OFF
  // iemOpt.rl2_opt = false; // OFF
  // iemOpt.rl3_opt = false; // OFF

  // // Initialize controller bill
  // for(int i = 0; i < 12; i++) {
  //   iemBill[i].nm = "";
  //   iemBill[i].Cn = "";
  //   iemBill[i].unit = 0;
  //   iemBill[i].tariff = 0;
  //   iemBill[i].enrg = 0;
  // }

  read_data_from_nvs("iem_conf", &iemConf, sizeof(iemConf));
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
}

float get_current(uint8_t adc_pin) {
  int sum = 0;
  for(int i = 0; i < 500; i++) {
    int adc = analogRead(adc_pin);
    float voltage = (adc / 4095.0) * 3.3; // Convert ADC value to voltage
    float centered = voltage - (3.3/2); // remove DC offset
    sum += centered * centered;
  }
  float rmsVoltage = sqrt(sum / 500); // Calculate RMS voltage
  float current = rmsVoltage * CALIBRATION_FACTOR; // Convert voltage to current using calibration factor
  return current;
}

void adc_read_currents() {
  // Read currents from ADC pins and update iemSta
  iemSta.c1_current = get_current(CURRENT1_ADC_PIN);
  iemSta.c2_current = get_current(CURRENT2_ADC_PIN);
  iemSta.c3_current = get_current(CURRENT3_ADC_PIN);
}
#endif // CONTROLLER_H