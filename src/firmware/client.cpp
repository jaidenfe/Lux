#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MESSAGE_SIZE 1024

using namespace std;

int sockfd;

struct sockaddr_in c_addr, s_addr;

char buffer[MESSAGE_SIZE];

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char const* argv[]) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd == -1) {
		cerr << "Failure to create client socket." << endl;
		return 1;
	}
	
	memset(&s_addr, 0, sizeof(s_addr));
	
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);
	
	string addr;
	
	if (argc == 1) {
		cout << "Address of server: ";
		cin >> addr;
		cout << endl;
	} else if (argc == 2) {
		addr = argv[1];
	} else {
		cerr << "Too many arguments provided in client execution. Attempting to continue anyways." << endl;
	}
	
	if (inet_pton(AF_INET, addr.c_str(), &s_addr.sin_addr) < 1) {
		cerr << "Could not convert given address to an address structure." << endl;
		return 1;
	}
	
	if (connect(sockfd, (const sockaddr*) &s_addr, sizeof(s_addr)) == -1) {
		cerr << "Socket error, could not connect to server." << endl;
		return 1;
	}
	
	cout << "Connected to server successfully, type \"exit\" to disconnect." << endl;
	
	pid_t pid = fork();
	
	if (pid == 0) {//child, send process
	
		string msg;
		char* c_msg;
	
		while(msg.compare("exit") != 0) {
			cin.getline(c_msg, MESSAGE_SIZE);//blocks until input
			msg = string(c_msg);
			
			if (msg.length() == 0) {
				msg = "\0";//to prevent disconnect after manually sent empty message
			}
			
			pthread_mutex_lock(&mtx);
			
			send(sockfd, msg.c_str(), msg.length(), 0);
			
			pthread_mutex_unlock(&mtx);
		}
		
		cout << "Terminated." << endl;
		return 0;
		
	} else if (pid > 0) {//parent, rcv process
	
		int len = 0;
		char msg[MESSAGE_SIZE];
		
		while(true) {
			pthread_mutex_lock(&mtx);
			len = recv(sockfd, msg, MESSAGE_SIZE, 0);//blocks until recieve
			pthread_mutex_unlock(&mtx);
			
			if (msg[0] == 0) {//server disconnected
				cout << "Connection terminated by server." << endl;
				return 0;
			}
			
			cout << "[Server]" << msg << endl;
			
			memset(&msg, 0, MESSAGE_SIZE);
		}
		
	} else {//error
		cerr << "Failed to fork client send/rcv processes." << endl;
		return 1;
	}
	
	return 0;
}