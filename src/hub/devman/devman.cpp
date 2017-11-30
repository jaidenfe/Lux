#include "devman.h"

#define MAX_NAME_LENGTH 20

//BEGIN EXTERNAL DEFINITIONS
map<string, DeviceGroup*> grps_n;
map<string, Device*> devs_ip;
map<string, Device*> devs_s;
//END EXTERNAL DEFINITIONS

char delim = '|';

//BEGIN Device
Device::Device(string ip, string name, string serial) {
	this->ip = ip;
	this->name = name;
	this->serial = serial;
	
	f_vers = "UNKNOWN";
	h_vers = "UNKNOWN";
	
	devs_ip.insert(pair<string, Device*>(this->ip, this));
	devs_s.insert(pair<string, Device*>(this->serial, this));
}

Device::~Device(){}


string Device::getIP() const {
	return ip;
}

string Device::getName() const {
	return name;
}

void Device::setName(string name) {
	this->name = name;
}

string Device::getSerial() const {
	return serial;
}

int Device::getLightLevel() const {
	return level;
}

void Device::setLightLevel(int level) {
	this->level = level;
}

string Device::firmware_version() const {
	return f_vers;
}

string Device::hardware_version() const {
	return h_vers;
}

void Device::set_f_vers(string f_vers) {
	this->f_vers = f_vers;
}

void Device::set_h_vers(string h_vers) {
	this->h_vers = h_vers;
}

string Device::toString() {
	return ip + delim + name + delim + serial + delim + to_string(level) + delim + f_vers + delim + h_vers;
}

bool Device::operator ==(const Device dev) const {
	return dev.getSerial().compare(this->serial) == 0;
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
	for (list<Device*>::iterator it = g_devs.begin(); it != g_devs.end(); ++it) {
		if ((*it) == *l) {
			return;//already in the group devices
		}
	}
	
	g_devs.push_back(l);
	devices++;
	
	cout << "Added device: " << l->getName() << endl;
}

bool DeviceGroup::removeDevice(Device* l) {
	if (*find(g_devs.begin(), g_devs.end(), l) == *g_devs.end()) {//no device found
		return false;
	}
	g_devs.remove(l);
	
	string name = l->getName();
	
	delete l;
	
	devices--;
	
	cout << "Remove device: " << name << endl;
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

Device* byIP(string ip) {
	if (devs_ip.count(ip) == 0) {//no device found
		return NULL;
	}
	return devs_ip[ip];
}

Device* bySerial(string serial) {
	if (devs_s.count(serial) == 0) {//no device found
		return NULL;
	}
	return devs_s[serial];
}

//BEGIN UTILITY FUNCTIONS
bool isProperLength(string name) {
	return name.length() <= MAX_NAME_LENGTH;
}

bool legalChars(string name, string legal) {
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
	string legal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_ ";
	return isProperLength(name) && legalChars(name, legal) && name.compare("NULL") != 0;//valid name
}

bool isValidGroupName(string name) {
	string legal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_ ";
	return isProperLength(name) && legalChars(name, legal) && name.compare("NULL") != 0;
	//valid name
}

bool isValidVersion(string vers) {
	string legal = ".1234567890";
	return isProperLength(vers) && legalChars(vers, legal);
}

DeviceGroup byGroupName(string name) {
	if (grps_n.count(name) == 0) {//no group found
		return DeviceGroup("NULL");//TODO return pointer
	}
	return *(grps_n[name]);
}

void device_for_each(dev_func* func, bool unique, void* aux) {
	set<Device*> checked;
	
	for (map<string, DeviceGroup*>::iterator it = grps_n.begin(); it != grps_n.end(); ++it) {
		DeviceGroup* g = it->second;

		list<Device*> devs = g->getDevices();

		for (list<Device*>::iterator dit = devs.begin(); dit != devs.end(); ++dit) {
			Device* d = *dit;
			
			if (unique && checked.count(d) > 0) {
				continue;
			}
			
			checked.insert(d);
			
			func(g, d, aux);
		}
	}
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
			
			file << "\t" << dev->toString() << endl;
			//    ID : IP : NAME : FV : HV
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

vector<string> split(string line, char delimiter){
    vector<string> list;
	string save = string(line);
    int pos = 0;
    while ((pos = save.find(delimiter)) != string::npos) {
        list.push_back(save.substr(0, pos));
        save.erase(0, pos + 1);
    }
    return list;
}
//END UTILITY FUNCTION

bool clearFile(string filename){
	ifstream file(filename, ios::out|ios::trunc);
	if(!file.is_open()){
		return false;
	}
	file.close();
	return true;
}

bool loadFile(string filename) {
	ifstream file;
	
	file.open(filename.c_str());
	
	string line;
	
	DeviceGroup* grp;
	
	int level;
	string ip, name, serial, f_vers, h_vers;
	
	while(getline(file, line)) {
		if (line[0] == '\t') {//device
			if (grp == NULL) {
				return false;//INCORRECT FORMATTING, DEVICE/S OUTSIDE OF GROUP
			}
			
			vector<string> data = split(line, delim);
			
			ip = trim(data[0]);
			name = trim(data[1]);
			serial = trim(data[2]);
			level = atoi(trim(data[3]).c_str());
			f_vers = trim(data[4]);
			h_vers = trim(line.substr(line.rfind(delim) + 1));
			
			Device* dev = new Device(ip, name, serial);
			
			dev->setLightLevel(level);
			dev->set_f_vers(f_vers);
			dev->set_h_vers(h_vers);
			
			grp->addDevice(dev);
		} else {//devicegroup
			string g_name = trim(line.substr(0, line.find("(")));
			if (grps_n.count(g_name) == 0) {
				grp = new DeviceGroup(g_name);
			} else {
				grp = grps_n[g_name];
			}
		}
	}
	
	return true;
}
