#include "json.h"

Json::Json(int icmd, std::string iuuid, std::string iserial){

	cmd = icmd;
	uuid = iuuid;
	serial = iserial;

	switch(cmd){
		case REGISTER:
			data["reg_key"]="";
			data["hardware_version"]="";
			data["firmware_version"]="";
			break;

		case CONNECT:
			data["hardware_version"]="";
			data["firmware_version"]="";
			break;

		case STATUS_REQUEST:
			// No data field required
			break;

		case STATUS:
			data["level"]="";
			break;

		case UPDATE_REQUEST:
			// devices are a comma seperated list of serial numbers
			// or the word "all" for all update
			data["devices"]="";
			// if name is specified the devices will be renamed the value here
			data["name"]="";
			// if add_group is not an empty string the devices will be added to the
			// specified group
			data["add_group"]="";
			// if remove_group is not an empty string the devices will be removed
			// from the specified group
			data["remove_group"]="";
			// if level is not an empty string the devices will be set to the specified level
			data["level"]="";
			break;

		case UPDATE:
			data["level"]="";
			break;

		case DISCONNECT_REQUEST:
			data["devices"]="";
			break;

		case DISCONNECT:
			// no data to send
			break;

		case UNREGISTER:
			// no data to send
			break;

		case FORCE_DISCONNECT:
			// maybe an error message?
			data["error"]="";
			break;

		case TEST:
			// Test message to send
			data["message"]="";
			break;

		default:
			std::cerr << "Error: Not a valid command." << std::endl;
	}

}

// takes a json string and turns it into a json object
Json::Json(std::string json_string){

	std::string temp;
	std::string temp2;

	// TODO: parse the string
	json_string = json_string.substr(1, json_string.length()-3);
	json_string = json_string.substr(json_string.find(':')+1);
	cmd = std::stoi(json_string.substr(0, json_string.find(',')));
	json_string = json_string.substr(json_string.find(':')+1);
	temp = json_string.substr(0, json_string.find(','));
	temp.erase(remove(temp.begin(), temp.end(), '\"'), temp.end());
	uuid = temp;
	json_string = json_string.substr(json_string.find(':')+1);
	temp = json_string.substr(0, json_string.find(','));
	temp.erase(remove(temp.begin(), temp.end(), '\"'), temp.end());
	serial = temp;
	json_string = json_string.substr(json_string.find('{')+1);
	// parse the data field
	while(json_string.find(',') != std::string::npos){
		temp = json_string.substr(0, json_string.find(':'));
		temp.erase(remove(temp.begin(), temp.end(), '\"'), temp.end());
		json_string = json_string.substr(json_string.find(':')+1);
		temp2 = json_string.substr(0, json_string.find(','));
		temp2.erase(remove(temp2.begin(), temp2.end(), '\"'), temp2.end());
		data[temp] = temp2;
		json_string = json_string.substr(json_string.find(',')+1);
	}
	temp = json_string.substr(0, json_string.find(':'));
	temp.erase(remove(temp.begin(), temp.end(), '\"'), temp.end());
	json_string = json_string.substr(json_string.find(':')+1);
	temp2 = json_string;
	temp2.erase(remove(temp2.begin(), temp2.end(), '\"'), temp2.end());
	data[temp] = temp2;
}

std::string Json::jsonify(){

	std::string json_string;

	json_string = "{\"cmd\":\""+std::to_string(cmd)+"\",\"uuid\":\""+uuid+"\",\"serial\":\""+serial+"\",\"data\":{";

	switch(cmd){
                case REGISTER:
                        if(data["reg_key"]!="") json_string = json_string + "\"reg_key\":\""+data["reg_key"]+"\",";
                        if(data["hardware_version"]!="") json_string = json_string + "\"hardware_version\":"+data["hardware_version"]+",";
                        if(data["firmware_version"]!="") json_string = json_string + "\"firmware_version\":"+data["firmware_version"];
                        json_string = json_string + "}";
                        break;

                case CONNECT:
                        if(data["hardware_version"]!="") json_string = json_string + "\"hardware_version\":"+data["hardware_version"]+",";
                        if(data["firmware_version"]!="") json_string = json_string + "\"firmware_version\":"+data["firmware_version"];
                        json_string = json_string + "}";
                        break;

                case STATUS_REQUEST:
                        // No data field required
                        json_string = json_string + "}";
                        break;

                case STATUS:
                        if(data["level"]!="") json_string = json_string + "\"level\":" + data["level"];
                        if(data["name"]!="") json_string = json_string+",\"name\":\""+data["name"]+"\"";
                        json_string = json_string + "}";
                        break;

                case UPDATE_REQUEST:
                        if(data["devices"]!="") json_string = json_string+"\"devices\":\""+data["devices"]+"\"";
                        if(data["name"]!="") json_string = json_string+",\"name\":\""+data["name"]+"\"";
                        if(data["add_group"]!="") json_string = json_string+",\"add_group\":\""+data["add_group"]+"\"";
                        if(data["remove_group"]!="") json_string = json_string+",\"remove_group\":\""+data["remove_group"]+"\"";
                        if(data["level"]!="") json_string = json_string+",\"level\":"+data["level"];
                        json_string = json_string + "}";
                        break;

                case UPDATE:
                        if(data["level"]!="") json_string = json_string + "\"level\":"+data["level"];
                        json_string = json_string + "}";
                        break;

                case DISCONNECT_REQUEST:
                        if(data["devices"]!="") json_string = json_string + "\"devices\":\""+data["devices"]+"\"";
                        json_string = json_string + "}";
                        break;

                case DISCONNECT:
                        // no data to send
                        json_string = json_string + "}";
                        break;

                case UNREGISTER:
                        // no data to send
                        json_string = json_string + "}";
                        break;

                case FORCE_DISCONNECT:
                        // maybe an error message?
                        if(data["error"]!="") json_string = json_string + "\"error\":\""+data["error"]+"\"";
                        json_string = json_string + "}";
                        break;

                case TEST:
                        // Test message to send
                        if(data["message"]!="") json_string = json_string + "\"message\":\""+data["message"]+"\"";
                        json_string = json_string + "}";
                        break;

                default:
                        std::cerr << "Error: Not a valid command." << std::endl;
                        json_string = "{\"error\":\"not a valid command\"}";
        }

	json_string = json_string + "}";

	return json_string;
}
