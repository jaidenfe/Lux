all: client server

server.o: firmware/server.cpp
	g++ -std=c++11 -c firmware/server.cpp

client.o: firmware/client.cpp
	g++ -std=c++11 -c firmware/client.cpp

server: server.o
	g++ server.o -o server
	rm server.o

client: client.o
	g++ client.o -o client
	rm client.o

clean:
	rm *.o *~ client server
