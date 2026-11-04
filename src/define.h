/* define.h */
#ifndef DEFINE_H
#define DEFINE_H

#define FIRMWARE_VERSION 111
// Pin definitions
#define MCU_SDA_PIN 8
#define MCU_SCL_PIN 9
#define BATT_ADC_PIN 1
#define PHASE1_ADC_PIN 2
#define PHASE2_ADC_PIN 3
#define PHASE3_ADC_PIN 4
#define CURRENT1_ADC_PIN 5
#define CURRENT2_ADC_PIN 6
#define CURRENT3_ADC_PIN 7
#define RELAY1_PIN 37
#define RELAY2_PIN 38
#define RELAY3_PIN 45
#define BUTTON_PIN 36
// MQTT and WiFi configuration
#define LCD_ADDR 0x27  // I2C address for 16x4 LCD display
#define LCD_COLS 20    // LCD display columns
#define LCD_ROWS 4     // LCD display rows
// Power calculation constants
#define POWER_CALCULATION_INTERVAL 5000 // Interval for power calculation in milliseconds
#define POWER_FACTOR 0.8 // Power factor for energy calculation
#define CALIBRATION_FACTOR 1.0 // Example calibration factor for current sensor
// WiFi configuration
#define DEFAULT_WIFI_STA_SSID "realme_swarna"
#define DEFAULT_WIFI_STA_PASSWORD "yatf6856"
// MQTT and WiFi configuration
#define MQTT_BROKER ""
#define MQTT_PORT 8883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_TOPIC "meter/status"
#define MQTT_SUBSCRIBE "meter/command"
// WiFi configuration
#define WIFI_AP_SSID "my_iem"
#define WIFI_AP_PASSWORD "iem@123456"
// Network status definitions
#define IDLE 0
#define WIFI_CONNECT 1
#define MQTT_INIT 2
#define MQTT_CONNECT 3
// Publish types
#define STATUS_UPDATE 1
#define BILL_UPDATE 2

#endif // DEFINE_H
