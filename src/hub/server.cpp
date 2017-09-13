#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <pthread.h>
#include <netdb.h>
#include <vector>

#define PORT 8080
#define READ_WAIT_PERIOD 1000000
//#define DEBUG

using namespace std;

struct ConnectionInfo {
	int sock;
	int server;
	int devices[20];
	struct sockaddr_in soc_address;
	int address_length;
	int device_num;
	int target_dev;
};

int server_sock;

bool auth;
bool ext;

//pthread_mutex_t recv_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex    = PTHREAD_MUTEX_INITIALIZER;

template <typename T> std::string toString(T t) {
	ostringstream ss;
	ss << t;
	return ss.str();
}

void setup(ConnectionInfo* av){

	int opt = 1;
	av->address_length = sizeof(av->soc_address);
	auth = false;
	av->device_num = 0;
	av->target_dev = -1;

	if((av->server = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		// Handle Exception
		pthread_mutex_lock( &print_mutex );
		std::cout << "Error creating socket" << std::endl;
		pthread_mutex_unlock( &print_mutex );
		return;
	}

	if (setsockopt(av->server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
		// Handle Exception
		pthread_mutex_lock( &print_mutex );
		std::cout << "Error setting socket options" << std::endl;
		pthread_mutex_unlock( &print_mutex );
		return;
	}

	av->soc_address.sin_family = AF_INET;
	av->soc_address.sin_addr.s_addr = INADDR_ANY;
	av->soc_address.sin_port = htons( PORT );

	if (bind(av->server, (struct sockaddr *)&av->soc_address, av->address_length)<0) {
		// Handle Exception
		pthread_mutex_lock( &print_mutex );
		std::cout << "Error binding port" << std::endl;
		pthread_mutex_unlock( &print_mutex );
		return;
	}
	
	server_sock = av->server;//keep track of server socket
}

void* listen(void* rl){
	struct ConnectionInfo *l = (struct ConnectionInfo*) rl;
	if (listen(l->server, 3) < 0) {
		// Handle Exception
		pthread_mutex_lock( &print_mutex );
		std::cout << "Error listening" << std::endl;
		pthread_mutex_unlock( &print_mutex );

		pthread_exit(0);
	}
	char buffer[1024] = {0};

	while(!ext){
		while(!auth){

			if ((l->devices[l->device_num] = accept(l->server, (struct sockaddr *)&l->soc_address, (socklen_t*)&l->address_length))<0) {
				// Handle Exception
				pthread_mutex_lock( &print_mutex );
				std::cout << "Error accepting connection" << std::endl;
				pthread_mutex_unlock( &print_mutex );
				pthread_exit(0);
			}

//			pthread_mutex_lock( &recv_mutex );
			int valread = recv(l->devices[l->device_num], buffer, 1024, 0);
//			pthread_mutex_unlock( &recv_mutex );

			if(std::string(buffer).compare("L000") == 0) {
				auth = true;
				printf("%s: Connect\n",buffer );
			}
			else {
				std::string msg = "Authentication Failure";
				printf("%s: %s\n",buffer, msg.c_str());
//				pthread_mutex_lock( &recv_mutex );
				send(l->devices[l->device_num], msg.c_str(), msg.length(), 0);
//				pthread_mutex_unlock( &recv_mutex );
			}
		}
		auth = false;

//		pthread_mutex_lock( &recv_mutex );
		send(l->devices[l->device_num], toString(l->device_num).c_str(), toString(l->device_num).length(), 0);
		// TODO: Add device to the device array at the next available location
//		pthread_mutex_unlock( &recv_mutex );
		l->device_num = l->device_num + 1;
	}
	pthread_exit(0);
}

void send(string ip, string data) {
	if (ip.compare("ALL") == 0 && send(server_sock, data.c_str(), data.length(), 0) == -1) {
		std::cerr << "Local error when sending data to client." << std::endl;
		return;
	}
	
	struct addrinfo hints;
	struct addrinfo* results;
	
	hints.ai_family = AF_INET;
	
	if (getaddrinfo(ip.c_str(), NULL, &hints, &results)) {//resolve host ip
		std::cerr << "Failed attempt to send message to client." << std::endl;
		return;
	}
	
	sockaddr_in client_addr = *((struct sockaddr_in*) results->ai_addr);//found client address
	client_addr.sin_port = htons(PORT);
	
	if (connect(server_sock, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in)) == -1) {
		std::cerr << "Connection to client failed." << endl;//errno updated
		return;
	}
	
	if (send(server_sock, data.c_str(), data.length(), 0) == -1) {
		cerr << "Local error when sending data to client." << endl;
		return;
	}
}

void broadcast(string data) {
	send("ALL", data);
}

void* read(void* rdn){
	struct ConnectionInfo *vi = (struct ConnectionInfo*) rdn;
	int target = vi->target_dev;

	while(!ext){
		char buf[2048] = {0};
		if(vi->devices[target] > 0){
#ifdef DEBUG
			pthread_mutex_lock( &print_mutex );
			cout << "[Device " << toString(target) << "] " << "There is a device" << endl;
			pthread_mutex_unlock( &print_mutex );
#endif
//			pthread_mutex_lock( &recv_mutex );
			int val = recv(vi->devices[target], buf, 2048, 0);
//			pthread_mutex_unlock( &recv_mutex );
#ifdef DEBUG
			pthread_mutex_lock( &print_mutex );
			cout << "[Device " << toString(target) << "] " << string(buf) << endl;
			pthread_mutex_unlock( &print_mutex );
#endif
			if(string(buf).compare("L000: Disconnect") == 0){
				pthread_mutex_lock( &print_mutex );
				cout << string(buf) << endl;
				pthread_mutex_unlock( &print_mutex );
				vi->device_num = vi->device_num - 1;
				vi->devices[target] = -1;
			} else if (string(buf).compare("TEST") == 0) {//TODO remove/comment
				pthread_mutex_lock(&print_mutex);
				
				broadcast("SUCCESS");
				
				pthread_mutex_unlock(&print_mutex);
			}
		}
#ifdef DEBUG
		else{
			pthread_mutex_lock( &print_mutex );
			cout << "[Device " << toString(target) << "] " << "No devices connected" << endl;
			pthread_mutex_unlock( &print_mutex );
		}
#endif

		usleep(READ_WAIT_PERIOD);
	}
	pthread_exit(0);
}

void* monitor(void* vv){
#ifdef DEBUG
	pthread_mutex_lock( &print_mutex );
	cout << "Start Monitoring" << endl;
	pthread_mutex_unlock( &print_mutex );
#endif
	struct ConnectionInfo *v = (struct ConnectionInfo*) vv;

#ifdef DEBUG
	pthread_mutex_lock( &print_mutex );
	cout << "Monitor Devices: " << toString(v->device_num) << endl;
	pthread_mutex_unlock( &print_mutex );
#endif

	pthread_t device_0, device_1, device_2, device_3, device_4, device_5;

	int rc;
	// TODO: Add dev number to struct and change it every time create new thread to read
	v->target_dev = 0;
	rc = pthread_create(&device_0, NULL, read, (void*)v);
	usleep(100);
	v->target_dev = 1;
	rc = pthread_create(&device_1, NULL, read, (void*)(v));
	usleep(100);
	v->target_dev = 2;
	rc = pthread_create(&device_2, NULL, read, (void*)(v));
	usleep(100);
	v->target_dev = 3;
	rc = pthread_create(&device_3, NULL, read, (void*)(v));
	usleep(100);
	v->target_dev = 4;
	rc = pthread_create(&device_4, NULL, read, (void*)(v));
	usleep(100);
	v->target_dev = 5;
	rc = pthread_create(&device_5, NULL, read, (void*)(v));
	usleep(100);

	rc = pthread_join(device_0, NULL);
	rc = pthread_join(device_1, NULL);
	rc = pthread_join(device_2, NULL);
	rc = pthread_join(device_3, NULL);
	rc = pthread_join(device_4, NULL);
	rc = pthread_join(device_5, NULL);

	pthread_exit(0);
}

int main(){

	struct ConnectionInfo *a = new ConnectionInfo();

	setup(a);

	int rc;

	pthread_t tlisten, tmonitor;
	rc = pthread_create(&tlisten, NULL, listen, (void *) a);
	// TODO: Check if rc is proper return code
	rc = pthread_create(&tmonitor, NULL, monitor, (void *) a);
	// TODO: Check if rc is proper return code

	rc = pthread_join(tlisten, NULL);
	rc = pthread_join(tmonitor, NULL);
	// TODO: Check rc values
	//std::thread tlisten (listen, (void*) a);
	//std::thread tmonitor (monitor);

	//tlisten.join();
	//tmonitor.join();

	return 0;
}
