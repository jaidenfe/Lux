#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

#define NETWORK_TIMEOUT 10
#define MAX_HUB_CONNECT_ATTEMPT 3
#define EEPROM_VAR_LEN 85

// Replace these with the JSON Library
#define REG_KEY "e7Cv2ro_8K"
#define REGISTER 0
#define CONNECT 1
#define STATUS_REQUEST 2
#define STATUS 3
#define STATUS_ACK 4
#define UPDATE 6
#define DISCONNECT_REQUEST 8
#define DISCONNECT 9
#define DISCONNECT_ACK 10
#define FORCE_DISCONNECT 13
#define TEST 14

// Serial Number defined by:
//   [L][HHH][PPPPPP][VV]
//      L - (1) - All Lux Serial Numbers must start with 'L'
//      H - (3) - Hardware Version Number (##.#) without decimal (numeric)
//      P - (6) - Production number (alpha-numeric, case-sensitive)
//      V - (2) - Device Code (case-insensitive/caps only) [Lux SmartSocket Device Code: 'SS'] 
#define SERIAL_NUM "L001000000SS"
#define HARDWARE_V "0.1"  // No actual hardware, just ESP8266 and LED
#define FIRMWARE_V "1.0"
#define LUX_FIRMWARE_V1_0
#define FIRMWARE_VERSION_CHECK(fv, check) fv==check
#define DEBUG


#ifdef LUX_FIRMWARE_V1_0  // Firmware Version 1.0 

// ESP Access Point Variables
const char* AP_ssid = "ESPSetup";
const char* AP_password = "ESP8266";

// Network Connection Variables
String LAN_ssid = "";
String LAN_pwrd = "";
String HUB_ip = "...";
int HUB_port = 0;

// State Variables
String mode = "WIFI_STA";
bool exit_ap = false;
bool hub_info = false;
bool Registered = false;
int Lux_status = 0;

// Networking Variables
ESP8266WebServer server(80); //Server on port 80
WiFiClient client;  // client to connect to the hub

// GPIO Pin Definition
const int enable_pin = 5;
const int indicator_pin = 4;

/*
 * This function saves network settings (ssid, password, HUB IP
 * address, and HUB port number) to the internal EEPROM of the
 * ESP8266 Wifi Module.
 */
void save_network_settings(){
  size_t i;
  char delim = '|';
  char raw[EEPROM_VAR_LEN];
  String data = "1|" + String(Registered) + delim + LAN_ssid + delim + LAN_pwrd + delim + HUB_ip + delim + String(HUB_port);

  // Read the stored values
  EEPROM.begin(512);
  for(i = 0; i < EEPROM_VAR_LEN; i++){
    EEPROM.get(i, raw[i]);
  }
  EEPROM.end();

  // Check if any new data needs to be stored
  if(String(raw) == data){
#ifdef DEBUG
    Serial.println("EEPROM: No new data to be saved");
#endif
    delay(100);
  }
  // New data so we are gonna write it to the EEPROM
  else{
#ifdef DEBUG
    Serial.println("EEPROM: Saving New Network Data");
#endif
    char var[EEPROM_VAR_LEN] = {'\0'};
    strncpy(var, data.c_str(), data.length());

    EEPROM.begin(512);
    for(i=0; i<EEPROM_VAR_LEN; i++){
      EEPROM.put(i, (uint8_t)data[i]);
    }
    EEPROM.commit();
    EEPROM.end();
  }
} 

/*
 * This function clears any saved network data by overwritting
 * previous data and seting the status byte to '0', signifying
 * there is no data saved
 */
void clear_network_settings(){
  size_t i;
  char delim = '|';
  String data = "0|";
  char var[EEPROM_VAR_LEN] = {'\0'};
  strncpy(var, data.c_str(), data.length());
  for(i=0; i<EEPROM_VAR_LEN; i++){
    EEPROM.write(i, (uint8_t)data[i]);
  }
  EEPROM.commit();
}

/*
 * This function reads the designated network settings portion
 * of the internal EEPROM, parses the data and, if the status
 * byte is not 0, saves the network information to the global
 * variables for ssid, password, hub ip address and hub port
 * number. If the status byte is 0, then the default network
 * settings will be used and the firmware will go into AP mode
 * to get new network data from the user.
 */
void load_network_settings(){
#ifdef DEBUG
  Serial.println("Loading Network Settings");
#endif
  size_t i;
  char delim = '|';
  char raw[EEPROM_VAR_LEN];

  EEPROM.begin(512);
  for(i = 0; i < EEPROM_VAR_LEN; i++){
    EEPROM.get(i, raw[i]);
  }
  EEPROM.end();
#ifdef DEBUG
  Serial.println("Recovered Credentials: " + String(raw));
#endif
  String data = String(raw);
  int stat = data.substring(0, data.indexOf(delim)).toInt();
  //Check if there is any saved network info
  if(stat){
    int ol = data.indexOf(delim);
    int r = data.substring(ol + 1, data.indexOf(delim, ol + 1)).toInt();
    if(r == 1) Registered = true;
    ol = data.indexOf(delim, ol + 1);
    LAN_ssid = data.substring(ol + 1, data.indexOf(delim, ol + 1));
    ol = data.indexOf(delim, ol + 1);
    LAN_pwrd = data.substring(ol + 1, data.indexOf(delim, ol + 1));
    ol = data.indexOf(delim, ol + 1);
    HUB_ip = data.substring(ol + 1, data.indexOf(delim, ol + 1));
    ol = data.indexOf(delim, ol + 1);
    HUB_port = data.substring(ol + 1).toInt();
  }
} 


/*
 * This function literally just PWMs an LED
 * to make it pulse to signal that configuation
 * is needed.
 */
void config_indicator(){
  analogWrite(indicator_pin, 0);
  delay(100);
  analogWrite(indicator_pin, 63);
  delay(30);
  analogWrite(indicator_pin, 127);
  delay(30);
  analogWrite(indicator_pin, 186);
  delay(30);
  analogWrite(indicator_pin, 255);
  delay(75);
  analogWrite(indicator_pin, 383);
  delay(75);
  analogWrite(indicator_pin, 512);
  delay(100);
  analogWrite(indicator_pin, 640);
  delay(100);
  analogWrite(indicator_pin, 767);
  delay(100);
  analogWrite(indicator_pin, 885);
  delay(200);
  analogWrite(indicator_pin, 1023);
  delay(150);
  analogWrite(indicator_pin, 885);
  delay(150);
  analogWrite(indicator_pin, 767);
  delay(100);
  analogWrite(indicator_pin, 640);
  delay(100);
  analogWrite(indicator_pin, 512);
  delay(100);
  analogWrite(indicator_pin, 383);
  delay(75);
  analogWrite(indicator_pin, 255);
  delay(75);
  analogWrite(indicator_pin, 186);
  delay(30);
  analogWrite(indicator_pin, 127);
  delay(30);
  analogWrite(indicator_pin, 63);
  delay(30);
  analogWrite(indicator_pin, 0);
  delay(400);
}


/*
 * This function blinks the indicator LEDs in
 * order to signify configuration success
 */
void config_success(){
  int del = 10;
  analogWrite(indicator_pin, 0);
  delay(del);
  analogWrite(indicator_pin, 1023);
  delay(del);
  analogWrite(indicator_pin, 0);
  delay(del*10);
  analogWrite(indicator_pin, 1023);
  delay(del);
  analogWrite(indicator_pin, 0);
}


/*
 * This rutine is executed when you open its IP address in 
 * a browser when connected to the ESP8266 AP. A webpage is
 * served with 
 */
void handleRoot() {
  // Serve the network configuration web page (s is the web page)
  int o1 = HUB_ip.indexOf(".");
  int o2 = HUB_ip.indexOf(".", o1 + 1);
  int o3 = HUB_ip.indexOf(".", o2 + 1);
  String s;
  if(!hub_info){
    s = "<!DOCTYPE html><html><head><title>LUX Configuration</title><style>body{font-family: sans-serif;background-color: #EAE7DC;}#settings{width: 40%;height: auto;color: #E85A4F;padding-top: 1%;padding-bottom: 2%;background-color: none;}.addr{width: 30px;}</style></head><body>"
             + String("<center><div id=\"settings\"><center><form name=\"configuration\"><h1>LUX Network Configuration</h1><hr>")
             + "SSID: <input type=\"text\" name=\"network_ssid\" value=\"" + LAN_ssid + "\"><br>"
             + "Password: <input type=\"password\" name=\"network_pwrd\" value=\"" + LAN_pwrd + "\"><br><br>"
             + "Hub IP Address: <input class=\"addr\" maxlength=\"3\" type=\"text\" name=\"hub_ip_0\" value=\"" + HUB_ip.substring(0, o1) + "\">"
             + ". <input class=\"addr\" maxlength=\"3\" type=\"text\" name=\"hub_ip_1\" value=\"" + HUB_ip.substring(o1+1, o2) + "\">"
             + ". <input class=\"addr\" maxlength=\"3\" type=\"text\" name=\"hub_ip_2\" value=\"" + HUB_ip.substring(o2+1, o3) + "\">"
             + ". <input class=\"addr\" maxlength=\"3\" type=\"text\" name=\"hub_ip_3\" value=\"" + HUB_ip.substring(o3+1) + "\">"
             + ": <input class=\"addr\" type=\"text\" name=\"hub_port\" value=\"" + String(HUB_port) + "\"><br><br>"
             + "<input name=\"Submit\"  type=\"submit\" value=\"Apply Configuration\"/>"
             + "</form></center></div></center></body></html>";
  }
  else{
    s = "<!DOCTYPE html><html><head><title>LUX Configuration</title><style>body{font-family: sans-serif;background-color: #EAE7DC;}#settings{width: 40%;height: auto;color: #E85A4F;padding-top: 1%;padding-bottom: 2%;background-color: none;}.addr{width: 30px;}</style></head><body>"
             + String("<center><div id=\"settings\"><center><form name=\"configuration\"><h1>LUX Network Configuration</h1><hr>")
             + "Hub IP Address: <input class=\"addr\" maxlength=\"3\" type=\"text\" name=\"hub_ip_0\" value=\"" + HUB_ip.substring(0, o1) + "\">"
             + ". <input class=\"addr\" maxlength=\"3\" type=\"text\" name=\"hub_ip_1\" value=\"" + HUB_ip.substring(o1+1, o2) + "\">"
             + ". <input class=\"addr\" maxlength=\"3\" type=\"text\" name=\"hub_ip_2\" value=\"" + HUB_ip.substring(o2+1, o3) + "\">"
             + ". <input class=\"addr\" maxlength=\"3\" type=\"text\" name=\"hub_ip_3\" value=\"" + HUB_ip.substring(o3+1) + "\">"
             + ": <input class=\"addr\" type=\"text\" name=\"hub_port\" value=\"" + String(HUB_port) + "\"><br><br>"
             + "<input name=\"Submit\"  type=\"submit\" value=\"Apply Configuration\"/>"
             + "</form></center></div></center></body></html>";
  }
  server.send(200, "text/html", s);
  
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
    Serial.println("\n***No New Network Information***");
#endif
  }
  else {
    // Get the network and hub information and store it globally
    if(!hub_info){
      LAN_ssid = server.arg("network_ssid");
      LAN_pwrd = server.arg("network_pwrd");
    }
    HUB_ip = server_ip;
    HUB_port = server.arg("hub_port").toInt();

    // Print out information to serial when debugging
#ifdef DEBUG
    Serial.println("\n\nNetwork: " + LAN_ssid);
    Serial.println("Password: " + LAN_pwrd);
    Serial.println("=======================================");
    Serial.println("Hub Address: " + HUB_ip + ":" + HUB_port);
#endif
    exit_ap = true;
    hub_info = false;
  }
}


/*
 * Start serial output and print out some information about the
 * Device like hardware version, firmware version, serial number
 * and what ever else we think might be useful in the future
 */
void setup(void) {
  WiFi.disconnect();
  // Initialize GPIO pin 5 (Triac Trigger/Enable Pin) as output
  pinMode(enable_pin, OUTPUT);
  // Initialize GPIO pin 4 (indicator led pin) as output
  pinMode(indicator_pin, OUTPUT);
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("Lux Home Automation Light Socket");
#ifndef DEBUG
  Serial.println("You should not be viewing this. Turn back now while you still have the chance.");
#endif
#ifdef DEBUG
  Serial.print("Serial Number: ");
  Serial.print(SERIAL_NUM);
  Serial.print("\nHardware Version: ");
  Serial.print(HARDWARE_V);
  Serial.print("\nFirmware Version: ");
  Serial.print(FIRMWARE_V);
  Serial.print("\n");
#endif
  // Load network ssid and password and hub ip address and port number
  load_network_settings();
}


/*
 * The main loop where all the magic happens!
 */
void loop(void) {
  // Station Mode: Attempt to connect to LAN
  if (mode == "WIFI_STA") {
    // Try to connect to network, If not available, start AP mode
    int network_disconnect = 0;
    const char* net_ssid = LAN_ssid.c_str();
    const char* net_pwrd = LAN_pwrd.c_str();
#ifdef DEBUG
    Serial.println("\n\nNetwork: " + LAN_ssid);
    Serial.println("Password: " + LAN_pwrd);
    Serial.println("=======================================");
    Serial.println("Hub Address: " + HUB_ip + ":" + HUB_port + "\n");
#endif
    WiFi.begin(net_ssid, net_pwrd);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
#ifdef DEBUG
      Serial.print(F("."));
#endif
      network_disconnect++;
      // Checking if network disconnect has reached network timeout
      if (network_disconnect == NETWORK_TIMEOUT) {
#ifdef DEBUG
        Serial.println("\nNetwork Connection Timed Out");
#endif
        WiFi.disconnect();
        // Change to AP Mode
        if (WiFi.mode(WIFI_AP)){
          mode = "WIFI_AP";
        }
        else{
#ifdef DEBUG
          Serial.println("Error Initializing AP Mode");
#endif
          delay(100);
        }
        break;
      }
    }
#ifdef DEBUG
    Serial.println("");
#endif
    // Main loop for LUX Client
    int hub_connection_attempts = 0;
    bool wait_for_response = false;
    while(WiFi.status() == WL_CONNECTED){
      save_network_settings();
      //delay(5000);
      if (client.connect(HUB_ip.c_str(), HUB_port)){
        // send register/connect command
        String initpack;
        if(!Registered){
          initpack = String(REGISTER) + "|" + REG_KEY + "|";
        }
        else{
          initpack = String(CONNECT) + "|Connecting|";
        }
        client.write(initpack.c_str());
#ifdef DEBUG
        Serial.println("Connection to Hub successful");
#endif
        while(client.connected()){
          // Checking that there is data to receive from server (Hub)
          if(client.available()){
            String data = client.readStringUntil('\n');
#ifdef DEBUG
            Serial.println("Received: " + data);
#endif
            // TODO: Turn data into JSON Object
            // TODO: Validate proper packet information (contains correct serial number, etc.)
            int cmd = data.substring(0, data.indexOf("|")).toInt();
            String payload = data.substring(data.indexOf("|") + 1);
            // switch cmd type
            String packet;
            switch(cmd){
              
              case STATUS_REQUEST:
                  // Send STATUS cmd and wait for STATUS_ACK or FORCE_DISCONNECT
                  if(!Registered){
                    Registered = true;
                  }
                  save_network_settings();
                  if(Lux_status > 0){
                    packet = String(STATUS) + "|ON|";
                  }
                  else{
                    packet = String(STATUS) + "|OFF|";
                  }
                  client.write(packet.c_str());

                  wait_for_response = true;
                  while(wait_for_response){
                    if(client.available()){
                      String ret = client.readStringUntil('\n');
                      int rcmd = ret.substring(0, ret.indexOf("|")).toInt();
                      switch(rcmd){
                        case STATUS_ACK:
                            // All is good in the world, you can terminate cmd routine
                            wait_for_response = false;
                            break;

                        case FORCE_DISCONNECT:
                            // Immediately terminate connection, something has gone terribly wrong!
#ifdef DEBUG
                            Serial.println("Received a FORCE_DISCONNECT Command.");
#endif
                            client.stop();
                            wait_for_response = false;
                            break;

                        default:
                            // Ignore what we got, resend the STATUS cmd
                            // TODO: Resend the STATUS cmd
#ifdef DEBUG
                            Serial.println("Got a different cmd than expected for response!");
#endif
                            delay(100);
                      }
                    }
                    else delay(500);
                  }
                  break;

                  
              case UPDATE:
                  // TODO: Update to use actual JSON utility
                  // Send STATUS cmd and wait for STATUS_ACK or FORCE_DISCONNECT
                  if(payload == "ON"){
                    digitalWrite(enable_pin, HIGH);
                    Lux_status = 10;
                  }
                  else{
                    digitalWrite(enable_pin, LOW);
                    Lux_status = 0;
                  }

                  packet = String(STATUS) + "|" + String(Lux_status) + "|";
                  client.write(packet.c_str());

                  wait_for_response = true;
                  while(wait_for_response){
                    if(client.available()){
                      String ret = client.readStringUntil('\n');
                      int rcmd = ret.substring(0, ret.indexOf("|")).toInt();
                      switch(rcmd){
                        case STATUS_ACK:
                            // All is good in the world, you can terminate cmd routine
                            wait_for_response = false;
                            break;

                        case FORCE_DISCONNECT:
                            // Immediately terminate connection, something has gone terribly wrong!
#ifdef DEBUG
                            Serial.println("Received a FORCE_DISCONNECT Command.");
#endif
                            client.stop();
                            wait_for_response = false;
                            break;

                        default:
                            // Ignore what we got, resend the STATUS cmd
                            // TODO: Resend STATUS cmd
#ifdef DEBUG
                            Serial.println("Got a different cmd than expected for response!");
#endif
                            delay(100);
                      }
                    }
                    else delay(500);
                  }     
                  break;


              case DISCONNECT_REQUEST:
                  // Send DISCONNECT and wait for DISCONNECT_ACK or FORCE_DISCONNECT
                  // then terminate client connection to Hub
                  packet = String(DISCONNECT) + "|Disconnecting|";
                  client.write(packet.c_str());

                  wait_for_response = true;
                  while(wait_for_response){
                    if(client.available()){
                      String ret = client.readStringUntil('\n');
                      int rcmd = ret.substring(0, ret.indexOf("|")).toInt();
                      switch(rcmd){
                        case DISCONNECT_ACK:
                            // All is good in the world, you can terminate cmd routine
                            client.stop();
                            wait_for_response = false;
#ifdef DEBUG
                            Serial.println("Disconnection Acknowledged");
#endif
                            break;

                        case FORCE_DISCONNECT:
#ifdef DEBUG
                            Serial.println("Received a FORCE_DISCONNECT Command.");
#endif
                            // Immediately terminate connection, something has gone terribly wrong!
                            client.stop();
                            wait_for_response = false;
                            break;

                        default:
                            // Ignore what we got, resend the STATUS cmd
#ifdef DEBUG
                            Serial.println("Got a different cmd than expected for response!");
#endif
                            // TODO: resend STATUS cmd
                            delay(100);
                      }
                    }
                    else delay(500);
                  }
                  break;
                  
              case FORCE_DISCONNECT:
#ifdef DEBUG
                  Serial.println("Received a FORCE DISCONNECT Command. Restarting Hub Connection.");
#endif
                  // Terminate client connection to Hub
                  client.stop();
                  break;
                  
              default:
                  // TODO Figure out what the defualt would be... there is an error, maybe disconnect
                  // from server and try to re-connect?
                  delay(100);
#ifdef DEBUG
                  Serial.println("ERROR: Unknown Command Type");
#endif
            }
          }
          else{
              delay(500);
          }
        }
        client.stop();
      }
      else{
        hub_connection_attempts++;
#ifdef DEBUG
        Serial.print("ERROR: Hub Unreachable. Attempting reconnect (");
        Serial.print(String(hub_connection_attempts));
        Serial.print("/");
        Serial.print(String(MAX_HUB_CONNECT_ATTEMPT));
        Serial.println(")");
#endif
        client.stop();
        if(hub_connection_attempts >= MAX_HUB_CONNECT_ATTEMPT){
#ifdef DEBUG
          Serial.println("ERROR: Unable to connect to Hub. Please connect to the Lux Config AP and input Hub IP and Port number.");
#endif
          WiFi.disconnect();
          if (WiFi.mode(WIFI_AP)){
            mode = "WIFI_AP";
          }
          else{
            // TODO: Throw an error, probably reboot the device
#ifdef DEBUG
            Serial.println("***Error Initializing Station Mode***");
#endif
          }
          hub_info = true;
        }
      }
    }
  }
  // AP Mode: Creates an Access Point and serves a network configuration webpage
  else if (mode == "WIFI_AP") {
    // Start AP Mode with ssid and password 
    // TODO: Fix the AP ssid and password so they are actually set
    WiFi.softAP(AP_ssid, AP_password);

    // Assign the static IP address of the server
    IPAddress myIP(192, 168, 4, 1);
#ifdef DEBUG
    Serial.print("Hot Spot IP:");
    Serial.println(myIP);
#endif
    // Assigning which routine will handle the root web location at the specified IP address 
    server.on("/", handleRoot);

    // Start the server
    server.begin();
#ifdef DEBUG
    Serial.println("HTTP server started");
#endif
    mode = "AP Client Mode";
  }
  // AP Client Mode: handles client connections
  else if (mode == "AP Client Mode") {
    // Pulsate an LED to let users know configuration is needed
    config_indicator();
    // Handle client connections  
    server.handleClient();
    // New network information has been provided
    // switch to Station Mode to attempt to connect
    if (exit_ap){
      WiFi.disconnect();
      if (WiFi.mode(WIFI_STA)){
        mode = "WIFI_STA";
      }
      else{
        // TODO: Implement Hardware Exceptions and Exception Handling
#ifdef DEBUG
        Serial.println("***Error Initializing Station Mode***");
#endif
      }
      exit_ap = false;
      // Turn an LED on and off twice to signify that the configuration was accepted
      delay(500);
      config_success();
    }
  }
  else{
    // TODO: Implement Hardware Exceptions and Exception Handling
#ifdef DEBUG
    Serial.println("Error: Unknown Mode");
#endif
  }
}

#endif    // END Firmware Version 1.0
