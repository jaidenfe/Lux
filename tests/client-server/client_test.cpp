#include "client.h"

int main(int argc, char* argv[]) {
	client_connect("127.0.0.1");
	
	//cout << "Connection attempted." << endl;
	
	while(!client_is_connected()) {
		client_safe_print("not connected");
	}
	
	//cout << "Checked and confirmed connection." << endl;
	
	/*
	client_safe_print("Enter something: ");
	
	client_safe_print(client_safe_read());
	
	cout << "Printed what was read." << endl;
	
	client_send("test");
	
	cout << "Test sent." << endl;
	
	string succ = client_recv();
	
	cout << "Test results recieved." << endl;
	
	client_safe_print(succ);
	
	cout << "Test results printed." << endl;
	*/
	
	pid_t pid = fork();
	
	if (pid == 0) {
		
		char* c_msg;
		string msg;
	
		while(msg.compare("exit") != 0) {
			cin.getline(c_msg, MESSAGE_SIZE);//blocks until input
			msg = string(c_msg);
		
			if (msg.length() == 0) {
				msg = "\0";//to prevent disconnect after manually sent empty message
			}
			client_send(msg);
		}
		
		client_disconnect();
		
	} else if (pid > 0) {
		
		while(true) {
			string msg = client_recv();
			
			if (!client_is_connected()) {
				break;
			}
			
			cout << "[Server]" << msg << endl;
		}
		
	} else {
		cerr << "Failed fork!" << endl;
	}
	
	cout << "Disconnected." << endl;
}