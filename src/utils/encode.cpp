#include "encode.h"

/*
*encode.cpp creates a JSON file with inputs from user. User must input a serial number, and a specific command
for this to work.
*Specific command has its assign value:
Command "on" will have device name, ip address, light status and light level.
*If the user did not assign value to a specific command, the JSON will output something empty.

*/
Encode::Encode(){
};
string Encode::stringfy(){
  string json;
  json = "{\"cmd\":"+this->command+",\"uuid\":"+to_string(this->uuid)+",\"Serial\":"+this->serial+",\"data\":{";
  if(this->command == "on"){
     json += "\"Device\":"+info.device+",\"IP\":"+info.ip+",\"light_status\":"+to_string(info.status)+",\"light_level\":"+to_string(info.level);
  }
  else if(this->command == "off"){
     json += "\"Device\":"+info.device+",\"IP\":"+info.ip+",\"light_status\":"+to_string(info.status)+",\"light_level\":"+to_string(info.level);
  }
  else if(this->command == "test"){
    json += "\"IP\":"+info.ip;
  }
  else if(this->command == "status"){
    json += "\"Device\":"+info.device+",\"IP\":"+info.ip+",\"light_status\":"+to_string(info.status)+",\"light_level\":"+to_string(info.level)+",\"Group\":"+info.group;
  }
  else if(this->command == "exit"){
    json += "\"IP\":"+info.ip;
  }
  else{
    json = "{\"cmd\":INVALID}";
    return json;
  }
  json += "}}";
  return json;
}

Encode::~Encode() {
	
}

void Encode::setUuid(int id){
  this->uuid = id;
}
void Encode::setIP(string newIP){
  this->info.ip = newIP;
}
void Encode::setLight_S(int newStatus){
  this->info.status = newStatus;
}
void Encode::setLight_L(int newLevel){
  this->info.level = newLevel;
}
void Encode::setGroupName(string newGroupN){
  this->info.group = newGroupN;
}
void Encode::setDeviceName(string newDeviceN){
  this->info.device = newDeviceN;
}
void Encode::setCommand(string command_string){
  this->command = command_string;
}
void Encode::setSerialN(string serial_input){
  this->serial = serial_input;
}
