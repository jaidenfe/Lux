#ifndef LUX_PARSER_H
#define LUX_PARSER_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

class Parser
{
private:
  std::map<std::string, std::string> dataMap;
  std::string command;
  std::string serial;
public:
  Parser(std::string json);
  ~Parser();
  std::string getIP()const;
  std::string getSerial()const;
  int getLight_S()const;
  int getLight_L()const;
  int getUUID()const;
  bool getValidation()const;
  std::string getGroupName()const;
  std::string getDeviceName()const;
  std::string getCommand()const;
};

#endif //LUX_PARSER_H
