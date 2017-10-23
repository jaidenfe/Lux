#include "../../src/hub/devman/devman.h"

int main() {
	///*
	DeviceGroup* grp1 = new DeviceGroup("TEST GROUP 1");
	
	Device* dev1 = new Device("192.168.1.1", "TEST DEVICE 1", "abc123");
	
	dev1->set_f_vers("F.VERS");
	dev1->set_h_vers("H.VERS");
	
	grp1->addDevice(dev1);
	
	Device* d1 = bySerial("abc123");
	
	updateFile("devices.dat");
	
	DeviceGroup* grp2 = new DeviceGroup("TEST GROUP 2");
	
	Device* dev2 = new Device("192.168.1.2", "TEST DEVICE 2", "def456");
	
	dev2->setLightLevel(8);
	
	grp1->addDevice(dev2);
	
	grp2->addDevice(dev1);
	
	string bad_name = "BAD/NAME*TEST_DEVICE#3";
	
	if (isValidName(bad_name)) {
		Device* dev3 = new Device("192.168.1.3", bad_name, "ghi789");
		
		grp1->addDevice(dev3);
	}
	
	updateFile("devices.dat");
	//*/
	
	///*
	loadFile("devices.dat");
	
	//cout << dev.getUUID() << " : " << dev.getIP() << " : " << dev.getName() << endl;
	
	updateFile("devices.dat");
	//*/
	
	return 0;
}