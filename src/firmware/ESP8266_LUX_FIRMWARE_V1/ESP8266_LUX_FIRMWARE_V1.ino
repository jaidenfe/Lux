#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <stdio.h>
//#import "index.h"

#define NETWORK_TIMEOUT 10
// Serial Number defined by:
//   [L][HHH][PPPPPP][V]
//      L - All Lux Serial Numbers must start with 'L'
//      H - Hardware Version Number (##.#) without decimal
//      P - Production number
//      V - Reserved for future use (default is 'T')
#define SERIAL_NUM "L010000000T"
#define HARDWARE_V "1.0"
#define LUX_FIRMWARE_V1_0

#define DEBUG

#ifdef LUX_FIRMWARE_V1_0

//SSID and Password of the ESP Access Point
const char* AP_ssid = "ESPSetup";
const char* AP_password = "ESP8266";

// SSID and Password to local network and ip and port to hub on the network
String LAN_ssid = "";
String LAN_pwrd = "";
String HUB_ip = "...";
int HUB_port = 0;
String mode = "WIFI_STA";
bool exit_ap = false;

ESP8266WebServer server(80); //Server on port 80

//==============================================================
//     This rutine is exicuted when you open its IP in browser
//==============================================================
void handleRoot() {
  Serial.println("Getting page");
  // Serve the network configuration web page
  String s = "<!DOCTYPE html><html><head><title>LUX Configuration</title><style>body{font-family: sans-serif;background-color: #EAE7DC;}#settings{width: 40%;height: auto;color: #E85A4F;padding-top: 1%;padding-bottom: 2%;background-color: none;}.addr{width: 40px;}</style></head><body>"
             + String("<center><div id=\"settings\"><center><form name=\"configuration\"><h1>LUX Network Configuration</h1><hr>")
             + "SSID: <input type=\"text\" name=\"network_ssid\" value=\"\"><br>"
             + "Password: <input type=\"password\" name=\"network_pwrd\" value=\"\"><br><br>"
             + "Hub IP Address: <input style=\"width: 40px;\" type=\"text\" name=\"hub_ip_0\" value=\"192\">"
             + ". <input style=\"width: 40px;\" type=\"text\" name=\"hub_ip_1\" value=\"168\">"
             + ". <input style=\"width: 40px;\" type=\"text\" name=\"hub_ip_2\" value=\"4\">"
             + ". <input style=\"width: 40px;\" type=\"text\" name=\"hub_ip_3\" value=\"6\">"
             + ": <input style=\"width: 40px;\" type=\"text\" name=\"hub_port\" value=\"80\"><br><br>"
             + "<input name=\"Submit\"  type=\"submit\" value=\"Apply Configuration\"/>"
             + "</form></center></div></center></body></html>";
  Serial.println("About to Serve Web Page");
  server.send(200, "text/html", s);
  Serial.println("Served Web Page");

  String server_ip = String(server.arg("hub_ip_0") + "." + server.arg("hub_ip_1") + "."
                       + server.arg("hub_ip_2") + "." + server.arg("hub_ip_3"));
  //Check if new network information is given
  if ((LAN_ssid == server.arg("network_ssid") || server.arg("network_ssid") == "") &&
      (LAN_pwrd == server.arg("network_pwrd") || server.arg("network_pwrd") == "") &&
      (HUB_ip == server_ip || server_ip == "...") &&
      (HUB_port == server.arg("hub_port").toInt() || server.arg("hub_port").toInt() == 0 )) {

#ifdef DEBUG
    Serial.println("\n\nNetwork: " + server.arg("network_ssid"));
    Serial.println("Password: " + server.arg("network_pwrd"));
    Serial.println("=======================================");
    String ip = String(server.arg("hub_ip_0") + "." + server.arg("hub_ip_1") + "."
                       + server.arg("hub_ip_2") + "." + server.arg("hub_ip_3"));
    Serial.println("Hub Address: " + ip + ":" + server.arg("hub_port"));
#endif
    Serial.println("\n***No New Network Information***");
  }
  else {
    // Get the network and hub information and store it globally
    LAN_ssid = server.arg("network_ssid");
    LAN_pwrd = server.arg("network_pwrd");
    HUB_ip = server.arg("hub_ip_0") + "." + server.arg("hub_ip_1") + "."
             + server.arg("hub_ip_2") + "." + server.arg("hub_ip_3");
    HUB_port = server.arg("hub_port").toInt();

    // Print out information to serial when debugging
#ifdef DEBUG
    Serial.println("\n\nNetwork: " + LAN_ssid);
    Serial.println("Password: " + LAN_pwrd);
    Serial.println("=======================================");
    Serial.println("Hub Address: " + HUB_ip + ":" + HUB_port);
#endif
    exit_ap = true;
  }

  
}

//===============================================================
//                  SETUP
//===============================================================
void setup(void) {
  WiFi.disconnect();
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("Lux Home Automation Light Socket");
  Serial.println("Firmware Version: 1.0");
  Serial.print("Hardware Version: ");
  Serial.print(HARDWARE_V);
  Serial.print("\n");
  Serial.print("Serial Number: ");
  Serial.print(SERIAL_NUM);
  Serial.print("\n");
}
//===============================================================
//                     LOOP
//===============================================================
void loop(void) {

  if (mode == "WIFI_STA") {
    Serial.println("Wifi Mode: " + mode);
    // Try to connect to network, If not available, start AP mode
    int network_disconnect = 0;
    const char* net_ssid = LAN_ssid.c_str();
    const char* net_pwrd = LAN_pwrd.c_str();
    WiFi.begin(net_ssid, net_pwrd);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(F("."));
      network_disconnect++;
      if (network_disconnect == NETWORK_TIMEOUT) {
        WiFi.disconnect();
        if (WiFi.mode(WIFI_AP)) mode = "WIFI_AP";          //Change to AP Mode
        else Serial.println("Error Initializing AP Mode");
        Serial.println("\nWifi Mode: " + mode);
        break;
      }
    }
  }
  else if (mode == "WIFI_AP") {
    WiFi.softAP(AP_ssid, AP_password);  //Start HOTspot removing password will disable security

    IPAddress myIP(192, 168, 4, 1);
    Serial.print("HotSpt IP:");
    Serial.println(myIP);

    server.on("/", handleRoot);      //Which routine to handle at root location

    server.begin();                  //Start server
    Serial.println("HTTP server started");
    mode = "Handle Client";
  }
  else if (mode == "Handle Client") {
    server.handleClient();
    if (exit_ap){
      WiFi.disconnect();
      if (WiFi.mode(WIFI_STA)) mode = "WIFI_STA";
      else Serial.println("***Error Initializing Station Mode***");
      exit_ap = false;
    }
  }
  else Serial.println("Error: Unknown Mode");
}

#endif
