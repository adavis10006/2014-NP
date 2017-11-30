#!/bin/sh
all: server

server: server.o safety_function.o pipe_list.o command_list.o
	g++ server.o safety_function.o pipe_list.o command_list.o -o server

server.o: server.h server.cpp
	g++ server.cpp -o server.o -c

safety_function.o: safety_function.h safety_function.cpp
	g++ safety_function.cpp -o safety_function.o -c

pipe_list.o: pipe_list.h pipe_list.cpp
	g++ pipe_list.cpp -o pipe_list.o -c

command_list.o: command_list.h command_list.cpp
	g++ command_list.cpp -o command_list.o -c

clean:
	rm -rf server.o safety_function.o pipe_list.o command_list.o server
