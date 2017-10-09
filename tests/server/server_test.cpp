#include "../../src/hub/server.h"

/*
void get_client_ip(const int c_fd, const string msg) {
	string response = "IP FOR " + to_string(c_fd) + " = " + client_ip_by_fd(c_fd);
	server_send(c_fd, response);
}
*/

int main(int argc, char* argv[]) {
	//server_commands["getip"] = get_client_ip;//Any Client->Server
	
	loadFile("../devman/devices.dat");
	
	cout << "Starting server..." << endl;
	
	server_start();
	
	cout << "Input enabled, searching for connections..." << endl;
	
	while(server_connections() == 0) {
		//do nothing
	}
	
	cout << "Connection found." << endl;
	
	while(true) {
		string s;
		cin >> s;
		
		if (s.compare("") == 0) {
			continue;
		}
		
		server_send(4, s);//first connection is always fd=4
		
		updateFile("../devman/devices.dat");
		
		if (s.compare("exit") == 0) {
			break;
		}
	}
	
	while(true) {
		//do nothing
	}
	
	return 0;
}