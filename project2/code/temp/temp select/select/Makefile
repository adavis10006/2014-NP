#!/bin/sh
all: server client delayclient

client: client.c
	gcc client.c -o client

delayclient: delayclient.c
	gcc delayclient.c -o delayclient
	
server: server.o client_list.o environment_list.o safety_function.o pipe_list.o command_list.o
	g++ server.o client_list.o environment_list.o safety_function.o pipe_list.o command_list.o -o server

server.o: server.h server.cpp
	g++ server.cpp -o server.o -c

client_list.o: client_list.h client_list.cpp
	g++ client_list.cpp -o client_list.o -c

environment_list.o: environment_list.h environment_list.cpp
	g++ environment_list.cpp -o environment_list.o -c
	
safety_function.o: safety_function.h safety_function.cpp
	g++ safety_function.cpp -o safety_function.o -c

pipe_list.o: pipe_list.h pipe_list.cpp
	g++ pipe_list.cpp -o pipe_list.o -c

command_list.o: command_list.h command_list.cpp
	g++ command_list.cpp -o command_list.o -c

clean:
	rm -rf server.o client_list.o environment_list.o safety_function.o pipe_list.o command_list.o server client
