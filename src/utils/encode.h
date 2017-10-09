#ifndef LUX_ENCODE_H
#define LUX_ENCODE_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

using namespace std;

class Encode
{
struct data{
  std::string group;
  std::string device;
  std::string ip;
  int status;
  int level;
};
private:
  data info;              //Making our struct data private
  int uuid;
  std::string serial;
  std::string command;
public:
  Encode();
  ~Encode();
  void setCommand(std::string com);
  void setSerialN(std::string serial_n);
  void setGroupName(std::string groupN);
  void setDeviceName(std::string deviceN);
  void setIP(std::string newIP);
  void setLight_S(int newStatus);
  void setLight_L(int newLevel);
  void setUuid(int id);
  std::string stringfy();
};

#endif //LUX_ENCODE_H
