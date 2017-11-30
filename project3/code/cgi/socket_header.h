#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__
//***********************************************
#include <arpa/inet.h>
#include <deque>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <string>
//***********************************************
#define CGI_VERSION "0.0.0"
//***********************************************
#define SA struct sockaddr
#define SERVER_PORT 5566
#define MAX_CLIENTS 50
#define BUFFER 1024
#define SOCKET_BUFFER 102400
//***********************************************
enum CLIENT_STATE
{
    CLIENT_NOT_EXIST,
    CLIENT_CLOSE,
    CLIENT_INITIAL,
    CLIENT_WAIT_READ,
    CLIENT_WAIT_WRITE
};
//***********************************************
#endif
