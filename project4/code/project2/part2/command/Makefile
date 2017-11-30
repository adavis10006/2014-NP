#!/bin/sh

all:
	g++ noop.cpp -o noop
	g++ number.cpp -o number
	g++ removetag.cpp -o removetag
	g++ removetag0.cpp -o removetag0
	g++ delayedremovetag.cpp -o delayedremovetag
	mv noop number removetag removetag0 delayedremovetag ../RAS/bin
	cp /bin/ls /bin/cat ../RAS/bin
	chmod 755 ../RAS/bin
