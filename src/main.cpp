#include <controller.h>
#include <server.h>

struct CONTROLLER_STATUS iemSta;
struct CONTROLLER_SETTINGS iemConf;

bool wifi_connected = false;
unsigned long wifi_timeout = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Controller Starting...\r\n");
  WiFi.mode(WIFI_AP_STA);

  // Start Access Point
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() == WL_CONNECTED && !wifi_connected) { 
    wifi_connected = true;
    Serial.println("WiFi Connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else  {
    wifi_connected = false;
    if (millis() - wifi_timeout > 5000) {// Retry every 5 seconds
      wifi_timeout = millis();
      Serial.println("Retry WiFi...");
      WiFi.disconnect();
      WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
    }
  }
}
