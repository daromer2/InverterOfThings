
/*************************************************************************************
/* ALl credits for some bits from https://github.com/scottwday/InverterOfThings
/* And i have trashed alot of his code, rewritten some. So thanks to him and credits to him. 
/*
/* oes no
/* https://www.youtube.com/channel/UCI6ASwT150rendNc5ytYYrQ?
/*************************************************************************************/


//TODO:
// Clean up webpages
// Fix update timer?
// Rewrite send to MQTT part so it sends json perhaps?

// Add some code so we can set stuff on the inverters.
// MPI should have the feedToGridCorrection so it can be set via mqtt


#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
//#include <espconn.h>
#include <EEPROM.h>
#include <PubSubClient.h>
//#include "uptime_formatter.h"
#include <ArduinoJson.h>

//#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>


#include "main.h"
#include "TickCounter.h"
#include "Settings.h"
#include "inverter.h"




//--------------------------------- Wifi State
#define CLIENT_NOTCONNECTED 0
#define CLIENT_RECONNECT 1
#define CLIENT_CONNECTING 2
#define CLIENT_PRECONNECTED 3
#define CLIENT_CONNECTED 4
byte currentApMode = 0;
byte requestApMode = WIFI_STA;
byte clientConnectionState = CLIENT_NOTCONNECTED;
bool clientReconnect = false;

//--------------------------------- IP connection
WiFiServer server(80);
WiFiClient client;
Settings _settings;
TickCounter _tickCounter;
int WIFI_COUNT = 0;

void serviceWifiMode();
void initHardware();

// Pages.cpp prototypes
void serve404(WiFiClient& client);
void serveWifiSetAp(WiFiClient& client, String req);
void serveWifiApList(WiFiClient& client);
void serveWifiSetupPage(WiFiClient& client);
void serveWifiSetAp(WiFiClient& client, String req);
void serveSetMqtt(WiFiClient& client, String req);
void serveMqtt(WiFiClient& client, String req);
void servePage(WiFiClient& client, String req);

//Local prototypes
bool sendtoMQTT();
void callback(char* top, byte* payload, unsigned int length);
void sendRaw();

extern QpigsMessage _qpigsMessage;
extern Qpigs55Message _qpigs55Message;
extern P003GSMessage _P003GSMessage;
extern P003PSMessage _P003PSMessage;
extern P006FPADJMessage _P006FPADJMessage;
extern String _nextCommandNeeded;
extern String _setCommand;
extern String _otherBuffer;

//---------------------- MQTT
PubSubClient mqttclient(client);

// Interface types that can be used. 
const byte MPI = 1;
const byte PCM = 0;
const byte PIP = 2;
const byte H55 = 3;
byte inverterType = H55; //And defaults in case...
String topic = "solar/";  //Default first part of topic. We will add device ID in setup
String st = "";

unsigned long mqtttimer = 0;
extern bool _allMessagesUpdated;
extern bool _otherMessagesUpdated;
//---------- LEDS  
//int Led_Red = 5;  //D1
//int Led_Green = 4;  //D2

void spiDisplayTask(void *parameter);
void invertterTask(void *parameter);


StaticJsonDocument<300> doc;  
//----------------------------------------------------------------------
void setup() 
{
 
  initHardware();
  _settings.load();
  serviceWifiMode();
  delay(2500);
  
  server.begin();
  delay(50);

  if (String(_settings._deviceType) == "MPI") {
    inverterType = MPI; }
  else if (String(_settings._deviceType) == "PIP"){
    inverterType = PIP; } 
  else if (String(_settings._deviceType) == "H55"){
    inverterType = H55; } 
  else {
     inverterType = PCM; 
  }
  
  //dev = _settings._deviceName.c_str();
  topic = topic + String(_settings._deviceName.c_str());
  
  mqttclient.setServer(_settings._mqttServer.c_str(), _settings._mqttPort);
  mqttclient.setCallback(callback);

  xTaskCreate(spiDisplayTask,   /* Task function. */
              "spiDisplayTask", /* String with name of task. */
              10000,            /* Stack size in words. */
              NULL,             /* Parameter passed as input of the task */
              2,                /* Priority of the task. */
              NULL);            /* Task handle. */

  xTaskCreate(invertterTask,   /* Task function. */
              "invertterTask", /* String with name of task. */
              10000,            /* Stack size in words. */
              NULL,             /* Parameter passed as input of the task */
              2,                /* Priority of the task. */
              NULL);            /* Task handle. */

  
  /*pinMode(Led_Red, OUTPUT); 
  pinMode(Led_Green, OUTPUT); 
  digitalWrite(Led_Red, HIGH); 
  digitalWrite(Led_Green, LOW); 
  */
  
  /*if (_settings._deviceName.length() > 0) 
    ArduinoOTA.setHostname(String("ESP-" + _settings._deviceName).c_str());
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();*/
}


void spiDisplayTask(void *parameter)
{
  while(1) sleep(100);
}

void invertterTask(void *parameter)
{
  while(1) {
    sendRaw();
  
    // Comms with inverter
    serviceInverter();  // Check if we recieved data or should send data
    sendtoMQTT();  // Update data to MQTT server if we should
   
      // Check if we have something to read from MQTT 
    mqttclient.loop();
  }
}


//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
void loop() 
{
  delay(10);

  // Make sure wifi is in the right mode
  serviceWifiMode();
  if (WiFi.status() == WL_CONNECTED) { //No use going to next step unless WIFI is up and running. 
    
    
    //ArduinoOTA.handle();  //Handle any OTA requests   DISABLED DUE TO BUG
  
    //If we have pending data to send send it first!
    sendRaw();
  
    // Comms with inverter
    serviceInverter();  // Check if we recieved data or should send data
    sendtoMQTT();  // Update data to MQTT server if we should
   
      // Check if we have something to read from MQTT 
    mqttclient.loop();
  }
  // Check if a client towards port 80 has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  if (req.indexOf("/wifi") != -1) {
    serveWifiSetupPage(client);    
  } else if (req.indexOf("/aplist") != -1) {
    serveWifiApList(client);
  } else if (req.indexOf("/setap") != -1) {
    serveWifiSetAp(client, req);
  } else if (req.indexOf("/mqtt") != -1) {
    serveMqtt(client, req);
  } else if (req.indexOf("/setmqtt") != -1) {
    serveSetMqtt(client, req);
  } else if (req.indexOf("/reboot") != -1) {
    serve404(client);
    delay(100);
    ESP.restart();
  } else {
    servePage(client, req);
    //serve404(client);
  }
  client.flush();
}

void initHardware()
{
 
  delay(100);
  //Wire.begin(4, 5);
  
  Serial.begin(115200); // Debugging towards UART1
  SerialSol.begin(2400, SERIAL_8N1, RXD2, TXD2);
  //SerialSol.begin(2400); // Using UART0 for comm with inverter. IE cant be connected during flashing

}

int WifiGetRssiAsQuality(int rssi)  // THis part borrowed from Tasmota code
{
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }
  return quality;
}

String qpigs55ToJson()
{
  String s;
    doc.clear();
    doc["solar1w"] =  _qpigs55Message.pv1P;
    doc["solar2w"] =  _qpigs55Message.pv2P;
    doc["solarInputV1"] =    _qpigs55Message.pv1V;
    doc["solarInputV2"] =    _qpigs55Message.pv2V;
    doc["battV"] =    _qpigs55Message.batt1V;
    doc["battCap"] =    _qpigs55Message.battCap;
    doc["/gridAcV"] = _qpigs55Message.gridAcV;
    doc["/gridAcP"] = _qpigs55Message.gridAcP;
    doc["/gridAcI"] = _qpigs55Message.gridAcI;
    doc["/gridAcF"] = _qpigs55Message.gridAcF;
    doc["/l1AcV"] = _qpigs55Message.l1AcV;
    doc["/l1AcP"] = _qpigs55Message.l1AcP;
    doc["/l1AcI"] = _qpigs55Message.l1AcI;
    doc["/l1AcF"] = _qpigs55Message.l1AcF;
    doc["/l1AcLp"] = _qpigs55Message.l1AcLp;
    
    serializeJson(doc,s);
    return(s);
}

bool sendtoMQTT() {
    if (millis() < (mqtttimer + 3000)) { 
    return false;
  }
  mqtttimer = millis();
  if (!mqttclient.connected()) {
    if (mqttclient.connect((String("ESP-" +_settings._deviceName)).c_str(), _settings._mqttUser.c_str(), _settings._mqttPassword.c_str() )) {
    
        Serial.println(F("Reconnected to MQTT SERVER"));
        mqttclient.publish((topic + String("/Info")).c_str(), ("{\"Status\":\"Im alive!\", \"DeviceType\": \"" + _settings._deviceType + "\",\"IP \":\"" + WiFi.localIP().toString() + "\"}" ).c_str());
        mqttclient.subscribe((topic + String("/code")).c_str());
        mqttclient.subscribe((topic + String("/code")).c_str());
      } else {
        Serial.println(F("CANT CONNECT TO MQTT"));
        //digitalWrite(Led_Green, LOW); 
        //delay(50);
        return false; // Exit if we couldnt connect to MQTT brooker
      }
  } 
    
    //mqttclient.publish((topic + String("/uptime")).c_str(), String("{\"human\":\"" + String(uptime_formatter::getUptime()) + "\", \"seconds\":" + String(millis()/1000) + "}").c_str()    );
    mqttclient.publish((topic + String("/wifi")).c_str()  , (String("{ \"FreeRam\": ") + String(ESP.getFreeHeap()) + String(", \"rssi\": ") + String(WiFi.RSSI()) + String(", \"dbm\": ") + String(WifiGetRssiAsQuality(WiFi.RSSI())) + String("}")).c_str());  


    
    Serial.print(F("Data sent to MQTT SERver"));
    Serial.print(F(" - up: "));
    //Serial.println(uptime_formatter::getUptime());
    //digitalWrite(Led_Green, HIGH);
    

  
  if (!_allMessagesUpdated) return false;
  
  _allMessagesUpdated = false; // Lets reset messages and process them
  
  if (inverterType == PCM) { //PCM
     mqttclient.publish((String(topic) + String("/battv")).c_str(), String(_qpigsMessage.battV).c_str());
     mqttclient.publish((String(topic) + String("/solarv")).c_str(), String(_qpigsMessage.solarV).c_str());
     mqttclient.publish((String(topic) + String("/batta")).c_str(), String(_qpigsMessage.battChargeA).c_str());
     mqttclient.publish((String(topic) + String("/wattage")).c_str(), String(_qpigsMessage.wattage).c_str());
     mqttclient.publish((String(topic) + String("/solara")).c_str(), String(_qpigsMessage.solarA).c_str());

    doc.clear();
    doc["battv"] =  _qpigsMessage.battV;
    doc["solarv"] = _qpigsMessage.solarV;
    doc["batta"] =  _qpigsMessage.battChargeA;
    doc["wattage"] =_qpigsMessage.wattage;
    doc["solara"] = _qpigsMessage.solarA;
    st = "";
    serializeJson(doc,st);
    mqttclient.publish((String(topic) + String("/status")).c_str(), st.c_str() );

     
  }
    /*if (inverterType == PIP) { //PIP
   
    doc.clear();
    doc["INFO"] =  "This one is not done...";
    st = "";
    serializeJson(doc,st);
    mqttclient.publish((String(topic) + String("/status")).c_str(), st.c_str() );

     
  }*/
  if (inverterType == H55) { 
    mqttclient.publish((String(topic) + String("/solar1w")).c_str(), String(_qpigs55Message.pv1P).c_str());
    mqttclient.publish((String(topic) + String("/solar2w")).c_str(), String(_qpigs55Message.pv2P).c_str());
    mqttclient.publish((String(topic) + String("/solarInputV1")).c_str(), String(_qpigs55Message.pv2V).c_str());
    mqttclient.publish((String(topic) + String("/solarInputV2")).c_str(), String(_qpigs55Message.pv2V).c_str());
    mqttclient.publish((String(topic) + String("/battV")).c_str(), String(_qpigs55Message.batt1V).c_str());
    mqttclient.publish((String(topic) + String("/battCap")).c_str(), String(_qpigs55Message.battCap).c_str());
  
   //mqttclient.publish((String(topic) + String("/battA")).c_str(), String(_qpigs55Message.battA).c_str());
    mqttclient.publish((String(topic) + String("/gridAcV")).c_str(), String(_qpigs55Message.gridAcV).c_str());
    mqttclient.publish((String(topic) + String("/gridAcP")).c_str(), String(_qpigs55Message.gridAcP).c_str());
    mqttclient.publish((String(topic) + String("/gridAcI")).c_str(), String(_qpigs55Message.gridAcI).c_str());
    mqttclient.publish((String(topic) + String("/gridAcF")).c_str(), String(_qpigs55Message.gridAcF).c_str());
    mqttclient.publish((String(topic) + String("/l1AcV")).c_str(), String(_qpigs55Message.l1AcV).c_str());
    mqttclient.publish((String(topic) + String("/l1AcP")).c_str(), String(_qpigs55Message.l1AcP).c_str());
    mqttclient.publish((String(topic) + String("/l1AcI")).c_str(), String(_qpigs55Message.l1AcI).c_str());
    mqttclient.publish((String(topic) + String("/l1AcF")).c_str(), String(_qpigs55Message.l1AcF).c_str());
    mqttclient.publish((String(topic) + String("/l1AcLp")).c_str(), String(_qpigs55Message.l1AcLp).c_str());
  
    st=qpigs55ToJson();
    mqttclient.publish((String(topic) + String("/status")).c_str(), st.c_str() );

  
  
  }

  if (inverterType == MPI) { //IF MPI
    mqttclient.publish((String(topic) + String("/solar1w")).c_str(), String(_P003GSMessage.solarInputV1*_P003GSMessage.solarInputA1).c_str());
    mqttclient.publish((String(topic) + String("/solar2w")).c_str(), String(_P003GSMessage.solarInputV2*_P003GSMessage.solarInputA2).c_str());
    mqttclient.publish((String(topic) + String("/solarInputV1")).c_str(), String(_P003GSMessage.solarInputV1).c_str());
    mqttclient.publish((String(topic) + String("/solarInputV2")).c_str(), String(_P003GSMessage.solarInputV2).c_str());
    mqttclient.publish((String(topic) + String("/solarInputA1")).c_str(), String(_P003GSMessage.solarInputA1).c_str());
    mqttclient.publish((String(topic) + String("/solarInputA2")).c_str(), String(_P003GSMessage.solarInputA2).c_str()); 
    mqttclient.publish((String(topic) + String("/battV")).c_str(), String(_P003GSMessage.battV).c_str());
    mqttclient.publish((String(topic) + String("/battA")).c_str(), String(_P003GSMessage.battA).c_str());

    mqttclient.publish((String(topic) + String("/acInputVoltageR")).c_str(), String(_P003GSMessage.acInputVoltageR).c_str());   
    mqttclient.publish((String(topic) + String("/acInputVoltageS")).c_str(), String(_P003GSMessage.acInputVoltageS).c_str());
    mqttclient.publish((String(topic) + String("/acInputVoltageT")).c_str(), String(_P003GSMessage.acInputVoltageT).c_str());

    mqttclient.publish((String(topic) + String("/acInputCurrentR")).c_str(), String(_P003GSMessage.acInputCurrentR).c_str());   
    mqttclient.publish((String(topic) + String("/acInputCurrentS")).c_str(), String(_P003GSMessage.acInputCurrentS).c_str());
    mqttclient.publish((String(topic) + String("/acInputCurrentT")).c_str(), String(_P003GSMessage.acInputCurrentT).c_str());

    mqttclient.publish((String(topic) + String("/acOutputCurrentR")).c_str(), String(_P003GSMessage.acOutputCurrentR).c_str());   
    mqttclient.publish((String(topic) + String("/acOutputCurrentS")).c_str(), String(_P003GSMessage.acOutputCurrentS).c_str());
    mqttclient.publish((String(topic) + String("/acOutputCurrentT")).c_str(), String(_P003GSMessage.acOutputCurrentT).c_str());

    mqttclient.publish((String(topic) + String("/acWattageR")).c_str(), String(_P003PSMessage.w_r).c_str());
    mqttclient.publish((String(topic) + String("/acWattageS")).c_str(), String(_P003PSMessage.w_s).c_str());
    mqttclient.publish((String(topic) + String("/acWattageT")).c_str(), String(_P003PSMessage.w_t).c_str());
    mqttclient.publish((String(topic) + String("/acWattageTotal")).c_str(), String(_P003PSMessage.w_total).c_str());
    mqttclient.publish((String(topic) + String("/ac_output_procent")).c_str(), String(_P003PSMessage.ac_output_procent).c_str());

    mqttclient.publish((String(topic) + String("/feedingGridDirectionR")).c_str(), String(_P006FPADJMessage.feedingGridDirectionR).c_str());
    mqttclient.publish((String(topic) + String("/calibrationWattR")).c_str(), String(_P006FPADJMessage.calibrationWattR).c_str());
    mqttclient.publish((String(topic) + String("/feedingGridDirectionS")).c_str(), String(_P006FPADJMessage.feedingGridDirectionS).c_str());
    mqttclient.publish((String(topic) + String("/calibrationWattS")).c_str(), String(_P006FPADJMessage.calibrationWattS).c_str());
    mqttclient.publish((String(topic) + String("/feedingGridDirectionT")).c_str(), String(_P006FPADJMessage.feedingGridDirectionT).c_str());
    mqttclient.publish((String(topic) + String("/calibrationWattT")).c_str(), String(_P006FPADJMessage.calibrationWattT).c_str());
    
    doc.clear();
    doc["solar1w"] =  _P003GSMessage.solarInputV1*_P003GSMessage.solarInputA1;
    doc["solar2w"] =  _P003GSMessage.solarInputV2*_P003GSMessage.solarInputA2;
    doc["solarInputV1"] =    _P003GSMessage.solarInputV1;
    doc["solarInputV2"] =    _P003GSMessage.solarInputV2;
    doc["solarInputA1"] =    _P003GSMessage.solarInputA1;
    doc["solarInputA2"] =    _P003GSMessage.solarInputA2;
    doc["battV"] = _P003GSMessage.battV;
    doc["battA"] = _P003GSMessage.battA;
    doc["acInputVoltageR"] = _P003GSMessage.acInputVoltageR;
    doc["acInputVoltageS"] = _P003GSMessage.acInputVoltageS;
    doc["acInputVoltageT"] = _P003GSMessage.acInputVoltageT;
    doc["acInputCurrentR"] = _P003GSMessage.acInputCurrentR;
    doc["acInputCurrentS"] = _P003GSMessage.acInputCurrentS;
    doc["acInputCurrentT"] = _P003GSMessage.acInputCurrentT;
    doc["acOutputCurrentR"] = _P003GSMessage.acOutputCurrentR;
    doc["acOutputCurrentS"] = _P003GSMessage.acOutputCurrentS;
    doc["acOutputCurrentT"] = _P003GSMessage.acOutputCurrentT;
    doc["acWattageR"] = _P003PSMessage.w_r;
    doc["acWattageS"] = _P003PSMessage.w_s;
    doc["acWattageT"] = _P003PSMessage.w_t;
    doc["acWattageTotal"] = _P003PSMessage.w_total;
    doc["acOutputProcentage"] = _P003PSMessage.ac_output_procent;
    doc["feedingGridDirectionR"] = _P006FPADJMessage.feedingGridDirectionR;
    doc["feedingGridDirectionS"] = _P006FPADJMessage.feedingGridDirectionS;
    doc["feedingGridDirectionT"] = _P006FPADJMessage.feedingGridDirectionT;
    doc["calibrationWattR"] = _P006FPADJMessage.calibrationWattR;
    doc["calibrationWattS"] = _P006FPADJMessage.calibrationWattS;
    doc["calibrationWattT"] = _P006FPADJMessage.calibrationWattT;
    st = "";
    serializeJson(doc,st);
    mqttclient.publish((String(topic) + String("/status")).c_str(), st.c_str() );
  }

  return true;
}


// Check if we have pending raw messages to send to MQTT. Then send it. 
void sendRaw() {
  if (_otherMessagesUpdated) {
    _otherMessagesUpdated = false;
    Serial.print("Sending other data to mqtt: ");
    Serial.println(_otherBuffer);
    mqttclient.publish((String(topic) + String("/debug/recieved")).c_str(), String(_otherBuffer).c_str() );
  }
}
/// TESTING MQTT SEND

void callback(char* top, byte* payload, unsigned int length) {
  Serial.println(F("Callback done"));
  if (strcmp(top,"pir1Status")==0){
    // whatever you want for this topic
  }
 
  String st ="";
  for (int i = 0; i < length; i++) {
    st += String((char)payload[i]);
  }
  
  mqttclient.publish((String(topic) + String("/debug/sent")).c_str(), String("top: " + topic + " data: " + st).c_str() );
  Serial.print(F("Current command: "));
  Serial.print(_nextCommandNeeded);
  Serial.print(F(" Setting next command to : "));
  Serial.println(st);
  _setCommand = st;
  // Add code to put the call into the queue but verify it firstly. Then send the result back to debug/new window?
}
