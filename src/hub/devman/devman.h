#ifndef LUX_DEVICE_MANAGER
#define LUX_DEVICE_MANAGER

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <map>
#include <list>

using namespace std;

class Device {
	private:
		long id;
		string ip, name;
	public:
		Device(long, string, string);
		~Device();
		long getID() const;
		string getIP() const;
		string getName() const;
		bool operator ==(const Device) const;//compares id, ip, name
};

class DeviceGroup {
	private:
		string name;
		int devices;
		list<Device*> g_devs;
	public:
		DeviceGroup(string);
		~DeviceGroup();
		void addDevice(Device*);
		bool removeDevice(Device*);
		int size();//# of devices
		list<Device*> getDevices();
		string getName();
};

extern map<string, DeviceGroup*> grps_n;
extern map<long, Device*> devs_id;
extern map<string, Device*> devs_ip;
extern map<string, Device*> devs_n;

Device byID(long);
Device byIP(string);
Device byName(string);

bool isValidName(string);//returns 0 (false) or 1 (true)
bool isValidGroupName(string);//same return policy

DeviceGroup byGroupName(string);

void updateFile(string);//must be called to update the devices file with current data

bool loadFile(string);

#endif //LUX_DEVICE_MANAGER