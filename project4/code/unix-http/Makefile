#!/bin/sh

all: http

safety_function.o: socket_header.h safety_function.h safety_function.cpp
	g++ safety_function.cpp -o safety_function.o -c

environment_list.o: safety_function.o environment_list.h environment_list.cpp
	g++ environment_list.cpp -o environment_list.o -c

http_handler.o: safety_function.o environment_list.o http_handler.h http_handler.cpp
	g++ http_handler.cpp -o http_handler.o -c

http.o: http_handler.o http.cpp
	g++ http.cpp -o http.o -c

http: http.o
	g++ safety_function.o environment_list.o http_handler.o http.o -o http

clean:
	rm -rf .*.swp safety_function.o environment_list.o http_handler.o http.o http

