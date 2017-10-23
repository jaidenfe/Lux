ROOT=../..
DEVMAN_FOLDER=$(ROOT)/src/hub/devman

FLAGS=-std=c++11

DEVMAN_FILES=$(DEVMAN_FOLDER)/devman.h $(DEVMAN_FOLDER)/devman.cpp

devman_test: $(DEVMAN_FILES) devman_test.cpp
	g++ $(FLAGS) -o devman_test.exe $(DEVMAN_FILES) devman_test.cpp
	
clean:
	rm -rf devman_test.exe