#include "server.h"

int sockfd;
bool running = false;

cmd_map server_commands;

set<int> reg_devs;
map<int, struct client*> conn_devs;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void server_send_dirty(int c_fd, string msg);
void* accept_devices(void* client_addr);
void* read_client(void* client_fd_ptr);
string uuid_gen();

//Client->Server
void client_exit(int client_fd, string message);
void client_test(int client_fd, string message);
void client_register(int client_fd, string message);
void client_connect(int client_fd, string message);
void client_status(int client_fd, string message);

//Web Client->Server
void client_unregister(int client_fd, string message);
void client_upd_req(int client_fd, string message);
void client_status_req(int client_fd, string message);

void server_start() {
	//Client->Server	
	server_commands[DISCONNECT] = client_exit;
	server_commands[TEST] = client_test;
	server_commands[REGISTER] = client_register;
	server_commands[CONNECT] = client_connect;
	server_commands[STATUS] = client_status;
	
	//Web Client->Server
	server_commands[UNREGISTER] = client_unregister;
	server_commands[UPDATE_REQUEST] = client_upd_req;
	server_commands[STATUS_REQUEST] = client_status_req;

	loadFile(DATA_FILE);

	new DeviceGroup("all");//create "all" group
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd == -1) {
		cerr << "Failure to create server socket." << endl;
		return;
	}
	
	struct sockaddr_in hints;
	
	hints.sin_family = AF_INET;
	hints.sin_addr.s_addr = INADDR_ANY;
	hints.sin_port = htons(PORT);
	
	if (bind(sockfd, (struct sockaddr*) &hints, sizeof(hints)) == -1) {
		cerr << "Could not assign a name to the server socket." << endl;
		return;
	}
	
	if (listen(sockfd, LISTEN_QUEUE_SIZE) == -1) {
		cerr << "Failed to listen for connections on port " << PORT << "." << endl;
		return;
	}
	
	pthread_t acc_dev;//create accept devices thread so the server can still send data concurrently
	
	int err = pthread_create(&acc_dev, NULL, accept_devices, (void*) &hints);
	
	if (err != 0) {
		cerr << "Failed to create \"acc_dev\" thread." << endl;
		return;
	}
}

int server_connections() {
	pthread_mutex_lock(&mtx);
	int size = conn_devs.size();
	pthread_mutex_unlock(&mtx);
	
	return size;
}

void server_send(int c_fd, string msg) {
	if (!running) {
		cerr << "Server attempted to send to client " << c_fd << " while not running." << endl;
		return;
	}
	
	if (msg.length() == 0) {
		cerr << "Attempted to send empty message to client " << c_fd << "." << endl;
		return;
	}
	
	server_send_dirty(c_fd, msg);
	
	if (conn_devs.count(c_fd) > 0) {//client client
		struct client* dev = conn_devs[c_fd];
		
		dev->status_wait = true;
		
		int attempts = 0;
		
		while(attempts < COMM_ATTEMPTS) {
			
			time_t s_time = time(NULL);//start time
		
			while((time(NULL) - s_time) < COMM_TIMEOUT) {
				if (!dev->status_wait) {
					return;//status recieved
				}
			}
		
			//no status recieved, resend
			server_send_dirty(c_fd, msg);
			attempts++;
		}
		
		//no status recieved, client exit
		client_exit(c_fd, "");
	}
}

void server_send_dirty(int c_fd, string msg) {
	/*
	This allows for a message of MESSAGE_SIZE - 1 (for null term) to be sent each time, meaning more bytes are sent,
	but there is no possibility that calling send() twice in rapid succession will concatenate messages in the buffer.
	*/
	
	char* a = new char[MESSAGE_SIZE];
	memcpy(a, msg.c_str(), MESSAGE_SIZE - 1);
	a[MESSAGE_SIZE - 1] = 0;
	cout << "[S->" << c_fd << "]" << msg << endl;
	if (send(c_fd, a, MESSAGE_SIZE, 0) == -1) {
		cerr << "Failed to send data to client " << c_fd << "." << endl;
		return;
	}
}

void* accept_devices(void* c_addr) {
	sockaddr_in hints = *(sockaddr_in*) c_addr;
	
	running = true;
	
	while(true) {
		int h_size = sizeof(hints);
		int c_fd = accept(sockfd, (struct sockaddr*) &hints, (socklen_t*) &h_size);
	
		if (c_fd == -1) {
			cerr << "Failed to establish a connection with a client." << endl;
			pthread_exit(0);
		}
	
		string ip = inet_ntoa(hints.sin_addr);
		
		pthread_t dev_rc;
		
		int err = pthread_create(&dev_rc, NULL, read_client, (void*) &c_fd);
	
		if (err != 0) {
			cerr << "Failed to create \"dev_rc\" thread for client at IP: " << ip << endl;
			pthread_exit(0);
		}
	}
	return NULL;
}

void* read_client(void* c_fd_p) {
	int c_fd = *(int*) c_fd_p;
	char msg[MESSAGE_SIZE];

	while(true) {
		memset(&msg, 0, MESSAGE_SIZE);//clear the buffer
		
		ssize_t bytes_rcvd = recv(c_fd, msg, MESSAGE_SIZE, 0);

		if (bytes_rcvd < 1) {
			cerr << "Client " << c_fd << " dropped the connection unexpectedly." << endl;
			client_exit(c_fd, "");
			return NULL;
		}
		string s_msg(msg);

		if (s_msg.front() != '{' || s_msg.back() != '}') {
			cerr << "Client " << c_fd << " sent a non-JSON (invalid) message to the server." << endl;
			client_exit(c_fd, "");
			return NULL;//not JSON
		}

		int cmd = Json(s_msg).cmd;//grab command type from JSON

        if (server_commands.count(cmd) == 0) {
            cerr << "Client " << c_fd << " issued unknown command \"" << cmd << "\"." << endl;
            client_exit(c_fd, "");
            return NULL;
        }

		pthread_mutex_lock(&mtx);
		map<int, cmd_func*>::iterator it = server_commands.find(cmd);
		pthread_mutex_unlock(&mtx);

		cout << "[S<-" << c_fd << "]: " << s_msg << endl;

		it->second(c_fd, s_msg);//respond accordingly
	}
	return NULL;
}

//REQUESTS:

void send_status_req(int c_fd, Device* d) {
	/*
	Json* json = new Json(STATUS_REQUEST, "0", d->getSerial());//TODO uuid
	server_send(c_fd, json->jsonify());//TO DEVICE
	*/

	server_send(c_fd, to_string(STATUS_REQUEST) + "|");
	//delete(json);
}

void send_status(int c_fd, Device* d, string group_name) {
	Json* json = new Json(STATUS, uuid_gen(), d->getSerial());
	json->data["name"] = d->getName();
	json->data["group_name"] = group_name;
	json->data["ip"] = d->getIP();
	json->data["level"] = "\"" + to_string(d->getLightLevel()) + "\"";
	//TODO add f_vers, h_vers
	string data = json->jsonify();
	//cout << data << endl;
	server_send(c_fd, data);
	delete(json);
}

//RESPONSES:
//CLIENT->SERVER
void client_exit(int c_fd, string msg) {	
	if (conn_devs.count(c_fd) == 0) {
		cerr << "Attempted to exit disconnected/invalid client " << c_fd << "." << endl;
		//delete(json);
		return;
	}
	
	struct client* dev = conn_devs[c_fd];
	
	/*
	if (changed_fds.count(c_fd) > 0) {//the fds never changed
		int fd = changed_fds[c_fd];
		fd_to_ser.insert(pair<int, string>(fd, serial));
		ser_to_fd.insert(pair<string, int>(serial, fd));
		changed_fds.erase(c_fd);//client reconnected to a different fd
	} else {
		by_ip.erase(ip);//if ip exists in the map
	}
	*/
	
	conn_devs.erase(c_fd);
	
	cout << "Client " << c_fd << " (" << dev->ip << ") disconnected." << endl;
    
    close(c_fd);
}

void client_test(int c_fd, string msg) {//client testing server NOT server->client
	server_send(c_fd, "SUCCESS");
}

void client_register(int c_fd, string msg) {
	Json* json = new Json(msg);

	if (json->data["reg_key"].compare(REG_KEY) != 0) {
		server_send(c_fd, to_string(FORCE_DISCONNECT));
		cerr << "Device " << c_fd << " attempted to register with improper reg key." << endl;
		delete(json);
		return;
	}

	if (reg_devs.count(c_fd) > 0) {
		client_connect(c_fd, msg);
		cerr << "Device " << c_fd << " attempted to register multiple times." << endl;
		delete(json);
		return;
	}
	
	reg_devs.insert(c_fd);

	/*
	string d_name = json->data["name"];

	if (!isValidName(d_name)) {
		cerr << "Invalid client name: " << d_name << endl;
		delete(json);
		return;
	}
	*/
    
    //string devip = client_ip_by_fd(c_fd);
	//struct client* dev = conn_devs[c_fd];
    
    //cout << devip << " reg at " << json->serial << endl;
	string default_name = "DEVICE-" + json->serial;
	Device* d = new Device("0"/*dev->ip*/, default_name, json->serial);//NO NAME, SET BY WEB CLIENT
	
	//d->setLightLevel(atoi(json->data["level"].c_str()));
	d->set_f_vers(json->data["firmware_version"]);
	d->set_h_vers(json->data["hardware_version"]);
	
	/*
	string g_name = "all";//TODO every client will be stored in the all group for now
	
	if (!isValidGroupName(g_name)) {
		cerr << "Invalid group name: " << g_name << endl;
		delete(json);
		delete(d);
		return;
	}
	
	DeviceGroup* g;

	if (grps_n.count(g_name) == 0) {//no group by that name exists
		g = new DeviceGroup(g_name);
	} else {
		DeviceGroup g_perm = byGroupName(g_name);
		g = &g_perm;
	}
	*/

	// TODO: Get rid of the next line and have the group created on start up and globally accessible
	//DeviceGroup* g = new DeviceGroup("all");//TODO more client groups than just 'all'
	
	byGroupName("all").addDevice(d);
	
	updateFile(DATA_FILE);
	
	client_connect(c_fd, msg);
	
	delete(json);
	
	//reg_devs.insert(json->serial);
	
	//send_status_req(c_fd, d);
	
	//delete(d);
}

void client_connect(int c_fd, string msg) {
	Json* json = new Json(msg);
	
	string serial = json->serial;
	
	//test all connections to make sure none have dropped unexpectedly
	
	for (map<int, struct client*>::iterator it = conn_devs.begin(); it != conn_devs.end(); ++it) {
		int fd = it->first;
		//struct client* cli = it->second;
		
		/*
		if (cli->serial.compare(serial) == 0) {//same serial, probably reset connection but it hasn't disconnected+exited yet
			//changed_fds.insert(pair<int, int>(fd, c_fd));
		}
		*/
		
		server_send(fd, to_string(TEST));
	}
	
	
	if (reg_devs.count(c_fd) == 0) {//not registered
		server_send(c_fd, to_string(REG_REQUEST));
		//cerr << "Device " << c_fd << " attempted to connect without registration." << endl;
		delete(json);
		return;
	}
	
	struct client* dev = (struct client*) malloc(sizeof(struct client));
	
	dev->fd = c_fd;
	dev->serial = serial;
	dev->ip = "0";//TODO
	dev->status_wait = false;
	
	Device* d = bySerial(serial);
	
	if (d == NULL) {
		cerr << "Device " << c_fd << " is registered but unknown to the client manager." << endl;
		client_exit(c_fd, "");
		delete(json);
		return;
	}
	
	/*
	string d_name = json->data["name"];
	
	if (!isValidName(d_name)) {
		cerr << "Invalid client name: " << d_name << endl;
		delete(json);
		return;
	}
	*/
	
	send_status_req(c_fd, d);
	
	delete(json);
}

void client_status(int c_fd, string msg) {
	Json* json = new Json(msg);
	
	string serial = json->serial;
	
	if (reg_devs.count(c_fd) == 0) {
		cerr << "Attempted to update the status of unregistered client" << c_fd << "." << endl;
		delete(json);
		return;
	}
	
	conn_devs[c_fd]->status_wait = false;
	
	//TODO change group if it's different
	
	Device* d = bySerial(serial);
	
	//d.setName(json->data["name"]);
	d->setLightLevel(atoi(json->data["level"].c_str()));
	//d.set_f_vers(json->data["firmware_version"]);
	//d.set_h_vers(json->data["hardware_version"]);
	
	for (map<int, struct client*>::iterator it = conn_devs.begin(); it != conn_devs.end(); ++it) {
		int fd = it->first;
		struct client* dev = it->second;
		
		if (dev->send_status) {
			send_status(fd, d, "all");//TODO group name
		
			it->second->send_status = false;
		}
	}
	
	updateFile(DATA_FILE);
	
	delete(json);
}

//WEB CLIENT->SERVER
void client_unregister(int c_fd, string msg) {
	Json* json = new Json(msg);
	
	string serial = json->serial;
	
	if (reg_devs.count(c_fd) == 0) {
		cerr << "Unregistered client " << c_fd << " attempted to unregister a client. " << endl;
		delete(json);
		return;
	}
	
	Device* d = bySerial(serial);
	
	string g_name = "all";//TODO the only group currently is all
	
	if (!isValidGroupName(g_name)) {
		cerr << "Invalid group name: " << g_name << endl;
		delete(json);
		return;
	}
	
	if (grps_n.count(g_name) == 0) {//no group by that name exists
		cerr << "Attempted to unregister a client (" << c_fd << ") from an invalid group." << endl;
		delete(json);
		return;
	}
	
	DeviceGroup g = byGroupName(g_name);
	
	g.removeDevice(d);
	
	free(conn_devs[c_fd]);
	
	reg_devs.erase(c_fd);
	conn_devs.erase(c_fd);
	
	updateFile(DATA_FILE);
	
	delete(json);
}

void client_upd_req(int c_fd, string msg) {
	Json* rcv_json = new Json(msg);
	
	string d_name = rcv_json->data["name"];
	string serial = rcv_json->serial;
    
    Device* d = bySerial(serial);
	
	if (d == NULL || reg_devs.count(c_fd) == 0) {//simulated response for invalid request
		cerr << "Client requested an update to the status of an unknown or disconnected client: " << serial << endl;
		
		//return unknown client status dummy message to web client
		Json* snd_json = new Json(STATUS, uuid_gen(), serial);
		snd_json->data["level"] = "0";
		snd_json->data["name"] = "UNKNOWN";
		
		server_send(c_fd, snd_json->jsonify());
		
		delete(rcv_json);
		delete(snd_json);
		return;
	}
	
	Json* snd_json = new Json(UPDATE, uuid_gen(), serial);
	
	string s_lev = rcv_json->data["level"];
	
	snd_json->data["level"] = s_lev;
	
	int level = atoi(s_lev.c_str());
    
	//!10 = OFF
	//10 = ON
	string type = "OFF";
	if (level == 10) {
		type = "ON";
	}
    
	struct client* dev = conn_devs[c_fd];
    
    pthread_mutex_lock(&mtx);
	server_send(dev->fd, to_string(UPDATE) + "|" + type);
	
	dev->send_status = true;
    pthread_mutex_unlock(&mtx);
	
	//updateFile(DATA_FILE);
	
	delete(rcv_json);
	delete(snd_json);
}

void client_status_req(int c_fd, string msg) { 
	for (map<string, DeviceGroup*>::iterator it = grps_n.begin(); it != grps_n.end(); ++it) {
		string g_name = it->first;
		DeviceGroup* g = it->second;
		
		list<Device*> devs = g->getDevices();
		
		//send a status command for each client
		for (list<Device*>::iterator dit = devs.begin(); dit != devs.end(); ++dit) {
			Device* d = *dit;
			
			send_status(c_fd, d, g_name);
		}
	}
	
	server_send(c_fd, STAT_REQ_DELIM);
}

string uuid_gen() {
    return "TODO";
}
