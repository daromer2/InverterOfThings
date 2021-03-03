#include <WiFi.h>
#include <Arduino.h>
#include "settings.h"

#define CLIENT_NOTCONNECTED 0
#define CLIENT_RECONNECT 1
#define CLIENT_CONNECTING 2
#define CLIENT_PRECONNECTED 3
#define CLIENT_CONNECTED 4

const char ApSsid[] = "SetSolar";

extern Settings _settings;
extern byte currentApMode;
extern byte requestApMode;
extern byte clientConnectionState;
extern bool clientReconnect;
extern int WIFI_COUNT;

//----------------------------------------------------------------------
// Configure wifi as access point to allow client config
void setupWifiAp()
{
  WiFi.mode(WIFI_AP);
  WiFi.disconnect();
  WiFi.softAP(ApSsid);
}

//----------------------------------------------------------------------
void setupWifiStation()
{
  WiFi.disconnect();
  delay(20);
  if (_settings._wifiSsid.length() == 0)
  {
    Serial.println(F("No client SSID set, switching to AP"));
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ApSsid);
  }
  else
  {
    Serial.print(F("Connecting to "));
    Serial.print(_settings._wifiSsid);
    Serial.print(":");
    Serial.println(_settings._wifiPass);
    WiFi.mode(WIFI_STA);
    if (_settings._deviceName.length() > 0)
      WiFi.setHostname(("ESP-" + _settings._deviceName).c_str());
    WiFi.begin(_settings._wifiSsid.c_str(), _settings._wifiPass.c_str());
  }
}

//----------------------------------------------------------------------
void serviceWifiMode()
{
  if (clientReconnect)
  {
    WiFi.disconnect();
    delay(10);
    clientReconnect = false;
    currentApMode = CLIENT_NOTCONNECTED;
  }
  
  if (currentApMode != requestApMode)
  {
    if (requestApMode == WIFI_AP)
    {
      Serial.println("Access Point Mode");
      setupWifiAp();             
      currentApMode = WIFI_AP;
    }

    if (requestApMode == WIFI_STA)
    {
      Serial.println("Station Mode");
      setupWifiStation();             
      currentApMode = WIFI_STA;
      clientConnectionState = CLIENT_CONNECTING;
    }
  }  

  if (clientConnectionState == CLIENT_CONNECTING)
  {    
    Serial.print(F("c:"));
    Serial.println(WIFI_COUNT);
    WIFI_COUNT++;
    sleep(1); // Else it will restart way to quickly.
    if (WIFI_COUNT > 500) { 
      WIFI_COUNT=0;
      ESP.restart();
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      clientConnectionState = CLIENT_CONNECTED;
      WIFI_COUNT = 0;
      Serial.print(F("IP address: "));
      Serial.println(WiFi.localIP());
    }
  }
}
