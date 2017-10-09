#include "Parser.h"

/*
Parser::Parser accepts a file name and extracts the entire file into a string.
After getting a single string of information, it stringstream it into a struct of data_name
and its data in order.
*/
Parser::Parser(std::string json){
  string data = "";
  int counter = 0;
  for(int i = 0;i<json.length();i++){
    char c =json[i];
    if(counter == 0 && (c == ':' || c == '"')){
      counter = 1;
    }
    else if(counter == 1 && (c == ',' || c == '"' || c == '{' || c == '}')){
      counter = 0;
      data = data + ' ';
    }
    if(counter == 1 && (c!=':'&& c!='"')){
      data = data + c;
    }
  };
  counter = 0;
  stringstream streamD(data);
  string dataI;
  string dataIN;
  while(streamD>>dataI){
    if(counter == 0 && dataI != "data"){
      dataIN = dataI;
      counter = 1;
    }
    else if(counter == 1){
      this->dataMap[dataIN] = dataI;
      counter = 0;
    }
  }
};

Parser::~Parser() {
	
}

string Parser::getSerial()const{
  return this->dataMap.at("Serial");
}
string Parser::getCommand()const{
  return this->dataMap.at("cmd");
}
int Parser::getLight_S()const{
  int x = stoi(this->dataMap.at("light_status"));
  return x;
}
int Parser::getLight_L()const{
  int x = stoi(this->dataMap.at("light_level"));
  return x;
}
int Parser::getUUID()const{
  int x = stoi(this->dataMap.at("uuid"));
  return x;
}
string Parser::getIP()const{
  return this->dataMap.at("IP");
}
string Parser::getGroupName()const{
  return this->dataMap.at("Group");
}
string Parser::getDeviceName()const{
  return this->dataMap.at("Device");
}
bool Parser::getValidation()const{
  string validation = this->dataMap.at("cmd");
  if(validation == "INVALID"){
    return false;
  }
  else{return true;}
}
