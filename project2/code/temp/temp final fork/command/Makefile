#!/bin/sh

all:
	g++ noop.cpp -o noop
	g++ number.cpp -o number
	g++ removetag.cpp -o removetag
	g++ removetag0.cpp -o removetag0
	mv noop number removetag removetag0 ../RAS/bin
	cp /bin/ls /bin/cat ../RAS/bin
	chmod u+x ../RAS/bin/cat ../RAS/bin/ls
