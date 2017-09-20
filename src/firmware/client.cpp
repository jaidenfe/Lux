#include "client.h"

int sockfd = -1;
bool connected = false;
struct sockaddr_in c_addr, s_addr;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void client_safe_print(string msg) {
	pthread_mutex_lock(&mtx);
	cout << msg << endl;
	pthread_mutex_unlock(&mtx);
}

string client_safe_read() {
	string msg;
	pthread_mutex_lock(&mtx);
	cin >> msg;
	pthread_mutex_unlock(&mtx);
	return msg;
}

bool client_connect(string addr) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd == -1) {
		cerr << "Failure to create client socket." << endl;
		return false;
	}
	
	memset(&s_addr, 0, sizeof(s_addr));
	
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);
	
	if (inet_pton(AF_INET, addr.c_str(), &s_addr.sin_addr) < 1) {
		cerr << "Could not convert given address to an address structure." << endl;
		return false;
	}
	
	if (connect(sockfd, (const sockaddr*) &s_addr, sizeof(s_addr)) == -1) {
		cerr << "Socket error, could not connect to server." << endl;
		return false;
	}
	
	connected = true;
	client_safe_print(client_recv());//recieve, print confirmation
	
	return true;
}

void client_disconnect() {
	if (!connected) {
		cerr << "Attempted to disconnect while not connected." << endl;
		return;
	}	
	
	client_send("exit");
	sockfd = -1;
	connected = false;
}

bool client_is_connected() {
	if (sockfd == -1) {
		connected = false;
		return false;
	}
	string msg = "test";
	char rcv[MESSAGE_SIZE];
	
	memset(&rcv, 0, MESSAGE_SIZE);
	
	pthread_mutex_lock(&mtx);
	send(sockfd, msg.c_str(), msg.length(), 0);
	recv(sockfd, rcv, 7/*SUCCESS*/, 0);
	pthread_mutex_unlock(&mtx);
	
	connected = rcv[0] != 0;
	
	return connected;
}

void client_send(string msg) {
	if (!connected) {
		cerr << "Attempted to send while not connected." << endl;
		return;
	}
	
	/*
	This allows for a message of MESSAGE_SIZE to be sent each time, meaning more bytes are sent,
	but there is no possibility that calling send() twice in rapid succession will combine messages.
	*/
	
	char* a = new char[MESSAGE_SIZE + 1];
	memcpy(a, msg.c_str(), MESSAGE_SIZE);
	a[MESSAGE_SIZE] = 0;
	
	pthread_mutex_lock(&mtx);	
	send(sockfd, a, MESSAGE_SIZE, 0);
	pthread_mutex_unlock(&mtx);
}

string client_recv() {
	if (!connected) {
		cerr << "Attempted to recieve while not connected." << endl;
		return "";
	}
	char msg[MESSAGE_SIZE];
	
	memset(&msg, 0, MESSAGE_SIZE);
	
	pthread_mutex_lock(&mtx);
	recv(sockfd, msg, MESSAGE_SIZE, 0);
	pthread_mutex_unlock(&mtx);
	
	if (msg[0] == 0) {//server disconnected
		cerr << "Server connection lost." << endl;
		
		sockfd = -1;
		connected = false;
	}

	return string(msg);
}