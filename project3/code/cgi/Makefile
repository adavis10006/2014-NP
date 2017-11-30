#!/bin/sh
WWW=/usr/lib/cgi-bin
#${HOME}/public_html

all: cgi
	cp cgi ${WWW}/hw3.cgi
	chmod 755 ${WWW}/hw3.cgi

communication.o: socket_header.h communication.h communication.cpp
	g++ communication.cpp -o communication.o -c

environment_list.o: communication.o environment_list.h environment_list.cpp
	g++ environment_list.cpp -o environment_list.o -c

multiclient.o: environment_list.o communication.o multiclient.h multiclient.cpp
	g++ multiclient.cpp -o multiclient.o -c

cgi.o: multiclient.o cgi.cpp
	g++ cgi.cpp -o cgi.o -c

cgi: cgi.o
	g++ communication.o environment_list.o multiclient.o cgi.o -o cgi

clean:
	rm -rf communication.o environment_list.o multiclient.o cgi.o cgi
	
