#include "devman.h"

#define MAX_NAME_LENGTH 20

//BEGIN EXTERNAL DEFINITIONS
map<string, DeviceGroup*> grps_n;
map<long, Device*> devs_id;
map<string, Device*> devs_ip;
map<string, Device*> devs_n;
//END EXTERNAL DEFINITIONS

//BEGIN Device
Device::Device(long id, string ip, string name) {
	this->id = id;
	this->ip = ip;
	this->name = name;
	
	devs_id.insert(pair<long, Device*>(this->id, this));
	devs_ip.insert(pair<string, Device*>(this->ip, this));
	devs_n.insert(pair<string, Device*>(this->name, this));
}

Device::~Device(){}

long Device::getID() const {
	return id;
}

string Device::getIP() const {
	return ip;
}

string Device::getName() const {
	return name;
}

bool Device::operator ==(const Device dev) const {
	return dev.getID() == this->id && dev.getIP() == this->ip && dev.getName().compare(this->name) == 0;
}
//END Device

//BEGIN DeviceGroup
DeviceGroup::DeviceGroup(string name) {
	this->name = name;
	
	devices = 0;
	
	grps_n.insert(pair<string, DeviceGroup*>(this->name, this));
}

DeviceGroup::~DeviceGroup(){}

void DeviceGroup::addDevice(Device* l) {
	g_devs.push_back(l);
	devices++;
}

bool DeviceGroup::removeDevice(Device* l) {
	if (*find(g_devs.begin(), g_devs.end(), l) == *g_devs.end()) {//no device found
		return false;
	}
	g_devs.remove(l);
	
	delete l;
	
	devices--;
	return true;
}

int DeviceGroup::size() {
	return devices;
}

list<Device*> DeviceGroup::getDevices() {
	return g_devs;
}

string DeviceGroup::getName() {
	return name;
}
//END DeviceGroup

Device byID(long id) {
	if (devs_id.count(id) == 0) {//no device found
		return Device(-1, "NULL", "NULL");//TODO throw custom exception?
	}
	return *devs_id[id];
}

Device byIP(string ip) {
	if (devs_ip.count(ip) == 0) {//no device found
		return Device(-1, "NULL", "NULL");
	}
	return *devs_ip[ip];
}

Device byName(string name) {
	if (devs_n.count(name) == 0) {//no device found
		return Device(-1, "NULL", "NULL");
	}
	return *devs_n[name];
}

//BEGIN UTILITY FUNCTIONS
bool isProperLength(string name) {
	return name.length() <= MAX_NAME_LENGTH;
}

bool legalChars(string name) {
	string legal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 ";
	for (int i = 0; i < legal.length(); i++) {
		if (name.find(legal[i]) != -1) {//string::npos = -1, return value of string::find() if no matches
			continue;
		}
		return false;//contains invalid character
	}
	return true;
}
//END UTILITY FUNCTIONS

bool isValidName(string name) {
	return isProperLength(name) && legalChars(name) && name != "NULL" && byName(name).getID() != -1;//valid name
}

bool isValidGroupName(string name) {
	return isProperLength(name) && legalChars(name) && name != "NULL" && byGroupName(name).getName() != "NULL";//valid name
}

DeviceGroup byGroupName(string name) {
	if (grps_n.count(name) == 0) {//no group found
		return DeviceGroup("NULL");
	}
	return *grps_n[name];
}

void updateFile(string filename) {
	ofstream file;
	
	file.open(filename.c_str(), ios::trunc);//creates file/*overwrites existing*
	
	list<string> used;
	
	for (map<string, DeviceGroup*>::iterator it = grps_n.begin(); it != grps_n.end(); ++it) {
		string grpn = it->first;
		DeviceGroup* grp = it->second;
		
		if (find(used.begin(), used.end(), grpn) == used.end()) {
			file << grpn << " (" << grp->size() << "):" << endl;//print group name and # devices if not printed
			used.push_back(grpn);
		}
		
		list<Device*> devs = grp->getDevices();
		
		for (list<Device*>::iterator dit = devs.begin(); dit != devs.end(); ++dit) {//print all devices in the group
			Device* dev = *dit;
			
			file << "\t" << dev->getID() << " : " << dev->getIP() << " : " << dev->getName() << endl;
		}
	}
	
	file.close();
}

//BEGIN UTILITY FUNCTION
string trim(string line) {
	string trimmed = line;
	while(trimmed[0] == ' ' || trimmed[0] == '\t') {
		trimmed = trimmed.substr(1);
	}
	while(trimmed[trimmed.length() - 1] == ' '|| trimmed[trimmed.length() - 1] == '\t') {
		trimmed = trimmed.substr(0, trimmed.length() - 1);
	}
	return trimmed;
}
//END UTILITY FUNCTION

bool loadFile(string filename) {
	ifstream file;
	
	file.open(filename.c_str());
	
	string line;
	
	DeviceGroup* grp;
	
	long id;
	string ip, name;
	
	while(getline(file, line)) {
		if (line[0] == '\t') {//device
			if (grp == NULL) {
				return false;//INCORRECT FORMATTING, DEVICE/S OUTSIDE OF GROUP
			}
			
			id = atol(trim(line.substr(0, line.find(":") - 1)).c_str());
			ip = trim(line.substr(line.find(":") + 1, line.rfind(":") - (line.find(":") + 1)));
			name = trim(line.substr(line.rfind(":") + 1));
			
			Device* dev = new Device(id, ip, name);
			
			grp->addDevice(dev);
		} else {//devicegroup
			grp = new DeviceGroup(trim(line.substr(0, line.find("("))));
		}
	}
}