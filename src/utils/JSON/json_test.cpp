#include <stdio.h>
#include "json.h"

int main(){
	Json Jobj = Json(CONNECT, "123abc", "L01000000SS");

	Jobj.data["hardware_version"] = "1.0";
	Jobj.data["firmware_version"] = "1.0";

	std::string j_string = Jobj.jsonify();

	std::cout << "JSON String: " << j_string << std::endl;

	Json Jobj2 = Json(j_string);

	std::cout << "\nJSON Command: " << std::to_string(Jobj2.cmd) << std::endl;
	std::cout << "JSON UUID: " << Jobj2.uuid << std::endl;
	std::cout << "JSON Serial: " << Jobj2.serial << std::endl;
	std::cout << "Data:" << std::endl;
	for(auto iit = Jobj2.data.cbegin(); iit != Jobj2.data.cend(); iit++){
                std::cout << "    " << iit->first << ": " << iit->second << std::endl;
        }

	Json Jobj3 = Json(UPDATE_REQUEST, "123abd", "L010000000SS");

	Jobj3.data["devices"] = "L010000001SS";
	Jobj3.data["level"] = "10";

	std::string j_string2 = Jobj3.jsonify();

	std::cout << "\nJSON String" << j_string2 << std::endl;

	Json Jobj4 = Json(j_string2);

	std::cout << "\nJSON Command: " << std::to_string(Jobj2.cmd) << std::endl;
        std::cout << "JSON UUID: " << Jobj2.uuid << std::endl;
        std::cout << "JSON Serial: " << Jobj2.serial << std::endl;
        std::cout << "Data:" << std::endl;
	for(auto it = Jobj4.data.cbegin(); it != Jobj4.data.cend(); it++){
		std::cout << "    " << it->first << ": " << it->second << std::endl;
	}
}
