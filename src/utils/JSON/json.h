#ifndef LUX_JSON_H
#define LUX_JSON_H

#define REGISTER 0
#define CONNECT 1
#define STATUS_REQUEST 2
#define STATUS 3
#define UPDATE_REQUEST 4
#define UPDATE 5
#define DISCONNECT_REQUEST 6
#define DISCONNECT 7
#define UNREGISTER 8
#define FORCE_DISCONNECT 9
#define TEST 10
#define REG_REQUEST 11

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <map>

class Json{

	public:
		int cmd;
		std::string uuid;
		std::string serial;
		std::map<std::string, std::string> data;

		Json(int icmd, std::string iuuid, std::string iserial);
		Json(std::string json_string);
		std::string jsonify();

};

#endif
