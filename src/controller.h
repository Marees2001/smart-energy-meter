
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>

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
// Controller function declarations

// Controller function declarations
void controller_init(void);
void controller_update(void);

#endif // CONTROLLER_H