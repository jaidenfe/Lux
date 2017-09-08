#include "../devman.h"

/* CORRECT OUTPUT:
 *
 *TEST GROUP 1 (2): 
 *	 99999 : 192.168.1.1 : TEST DEVICE 1
 *	 99998 : 192.168.1.2 : TEST DEVICE 2
 *TEST GROUP 2 (1): 
 *	 99999 : 192.168.1.1 : TEST DEVICE 1
 *
*/

int main() {
	///*
	DeviceGroup* grp1 = new DeviceGroup("TEST GROUP 1");
	
	Device* dev1 = new Device(99999, "192.168.1.1", "TEST DEVICE 1");
	
	grp1->addDevice(dev1);
	
	Device d1 = byName("TEST DEVICE");
	
	updateFile("devices.dat");
	
	DeviceGroup* grp2 = new DeviceGroup("TEST GROUP 2");
	
	Device* dev2 = new Device(99998, "192.168.1.2", "TEST DEVICE 2");
	
	grp1->addDevice(dev2);
	
	grp2->addDevice(dev1);
	
	string bad_name = "BAD/NAME*TEST_DEVICE#3";
	
	if (isValidName(bad_name)) {
		Device* dev3 = new Device(99997, "192.168.1.3", bad_name);
		
		grp1->addDevice(dev3);
	}
	
	updateFile("devices.dat");
	//*/
	
	/*
	loadFile("devices.dat");
	
	//Device dev = byID(99999l);
	//cout << dev.getID() << " : " << dev.getIP() << " : " << dev.getName() << endl;
	
	updateFile("devices.dat");
	//*/
	
	return 0;
}