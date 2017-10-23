ROOT=../..
HUB_FOLDER=$(ROOT)/src/hub
JSON_FOLDER=$(ROOT)/src/utils/JSON
DEVMAN_FOLDER=$(HUB_FOLDER)/devman

FLAGS=-std=c++11 -pthread -Wall

ALL_FILES=$(JSON_FOLDER)/json.h $(DEVMAN_FOLDER)/devman.h $(HUB_FOLDER)/server.h $(JSON_FOLDER)/json.cpp $(DEVMAN_FOLDER)/devman.cpp $(HUB_FOLDER)/server.cpp server_test.cpp 

server_test: $(ALL_FILES)
	g++ $(FLAGS) $(ALL_FILES) -o server_test.exe
	
clean:
	rm -rf server_test.exe