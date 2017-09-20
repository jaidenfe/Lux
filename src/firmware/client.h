#ifndef LUX_CLIENT_H
#define LUX_CLIENT_H

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

void client_safe_print(string message);
string client_safe_read();
bool client_connect(string address);
void client_disconnect();
bool client_is_connected();
void client_send(string message);
string client_recv();

#endif //LUX_CLIENT_H