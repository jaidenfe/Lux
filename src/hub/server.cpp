#include <iostream>
#include <unistd.h>
#include <string.h>
#include <map>
#include <list>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define LISTEN_QUEUE_SIZE 5
#define MESSAGE_SIZE 1024

using namespace std;

int sockfd;

typedef void cmd_func(const int c_fd, const string msg);
typedef map<string, cmd_func*> cmd_map;

cmd_map commands;
map<string, int> by_ip;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* accept_devices(void* c_addr);
void* read_client(void* c_fd_p);
void client_exit(int c_fd, string msg);

int main(int argc, char const* argv[]) {
	commands["exit"] = client_exit;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd == -1) {
		cerr << "Failure to create server socket." << endl;
		return 1;
	}
	
	struct sockaddr_in hints;
	
	hints.sin_family = AF_INET;
	hints.sin_addr.s_addr = INADDR_ANY;
	hints.sin_port = htons(PORT);
	
	if (bind(sockfd, (struct sockaddr*) &hints, sizeof(hints)) == -1) {
		cerr << "Could not assign a name to the server socket." << endl;
		return 1;
	}
	
	if (listen(sockfd, LISTEN_QUEUE_SIZE) == -1) {
		cerr << "Failed to listen for connections on port " << PORT << "." << endl;
		return 1;
	}
	
	//create accept devices thread so the server can still send data concurrently
	
	pthread_t acc_dev;
	
	int err = pthread_create(&acc_dev, NULL, accept_devices, (void*) &hints);
	
	if (err != 0) {
		cerr << "Failed to create \"acc_dev\" thread." << endl;
		return 1;
	}
	
	err = pthread_join(acc_dev, NULL);
	
	if (err != 0) {
		cerr << "Failed to join \"acc_dev\" thread." << endl;
		return 1;
	}
	
	return 0;
}

void* accept_devices(void* c_addr) {
	sockaddr_in hints = *(sockaddr_in*) c_addr;
	
	while(true) {
		int h_size = sizeof(hints);
		int c_fd = accept(sockfd, (struct sockaddr*) &hints, (socklen_t*) &h_size);//TODO correct size? blocks until connection is found?
	
		if (c_fd == -1) {
			cerr << "Failed to establish a connection with a client." << endl;
			pthread_exit(0);
		}
	
		string ip = inet_ntoa(hints.sin_addr);
		
		cout << "Device " << c_fd << " connected from IP: " << ip << endl;
		
		string msg = "Connection established.";
		send(c_fd, msg.c_str(), msg.length(), 0);
		
		pthread_mutex_lock(&mtx);
		by_ip.insert(pair<string, int>(ip, c_fd));
		pthread_mutex_unlock(&mtx);
		
		pthread_t dev_rc;
		
		int err = pthread_create(&dev_rc, NULL, read_client, (void*) &c_fd);
	
		if (err != 0) {
			cerr << "Failed to create \"dev_rc\" thread for device at IP: " << ip << endl;
			pthread_exit(0);
		}
	
		err = pthread_join(dev_rc, NULL);
	
		if (err != 0) {
			cerr << "Failed to join \"dev_rc\" thread for device at IP: " << ip << endl;
			pthread_exit(0);
		}
	}
}

list<string> split(string line, char delimiter){
    list<string> pieces;
	string save = string(line);
    int pos = 0;
    while ((pos = save.find(delimiter)) != string::npos) {
        pieces.push_back(save.substr(0, pos));
        save.erase(0, pos + 1);
    }
    return pieces;
}

void* read_client(void* c_fd_p) {
	int c_fd = *(int*) c_fd_p;
	
	int len = 0;
	char msg[MESSAGE_SIZE];
	
	while(true) {
		len = recv(c_fd, msg, MESSAGE_SIZE, 0);
		
		string s_msg(msg);
		
		if (s_msg.length() == 0) {//client is disconnected, the buffer is just being read constantly
			client_exit(c_fd, "");
		}
		
		memset(&msg, 0, MESSAGE_SIZE);//clear the buffer
		list<string> pieces = split(s_msg, ' ');//split on whitespace
		string key = pieces.front();
		
		pthread_mutex_lock(&mtx);
		
		if (commands.count(key) == 0) {
			cout << "[Device " << c_fd << "]" << s_msg << endl;
		} else {
			commands.find(key)->second(c_fd, s_msg);
		}
		
		pthread_mutex_unlock(&mtx);
	}
}

void client_exit(const int c_fd, const string msg) {
	cout << "Device " << c_fd << " disconnected." << endl;
	pthread_exit(0);
}