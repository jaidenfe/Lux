#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 8080
#define SERIAL_NUM "L000"

int main(int argc, char const *argv[]) {

	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		// Exception handling
		std::cout << "Exception creating socket" << std::endl;
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
		std::cout << "Invalid address" << std::endl;
		return -1;
	}

	if (connect(sock, (const sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		std::cout << "\nConnection Failed \n" << std::endl;
		return -1;
	}

	send(sock , SERIAL_NUM , strlen(SERIAL_NUM) , 0 );
	valread = recv(sock, buffer, 1024, 0);
	printf("%s\n",buffer );

	while(getchar() != '\n');

	std::string msg = std::string(SERIAL_NUM) + ": Disconnect";
	send(sock, msg.c_str(), strlen(msg.c_str()), 0);

	return 0;
}
