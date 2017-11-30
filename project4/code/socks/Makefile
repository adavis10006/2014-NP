#/bin/sh

all: server

server: safety_function.o firewall.o socks4.o server.o
	g++ safety_function.o firewall.o socks4.o server.o -o server

server.o: socks4.o socket_header.h server.cpp
	g++ server.cpp -o server.o -c 

socks4.o: firewall.o socket_header.h socks4.h socks4.cpp
	g++ socks4.cpp -o socks4.o -c

firewall.o: safety_function.o firewall.h firewall.cpp
	g++ firewall.cpp -o firewall.o -c

safety_function.o: socket_header.h safety_function.h safety_function.cpp
	g++ safety_function.cpp -o safety_function.o -c

clean:
	rm -rf .*.swp safety_function.o firewall.o socks4.o server.o server
