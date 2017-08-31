all: bin/client bin/server

server.o: firmware/server.cpp
	g++ -std=c++11 -c firmware/server.cpp

client.o: firmware/client.cpp
	g++ -std=c++11 -c firmware/client.cpp

bin/server: server.o
	mkdir -p bin
	g++ server.o -o bin/server
	rm server.o

bin/client: client.o
	mkdir -p bin
	g++ client.o -o bin/client
	rm client.o

clean:
	rm -rf bin
