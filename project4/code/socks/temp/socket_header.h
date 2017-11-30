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
#define SOCKS4_VERSION "0.0.0"
//***********************************************
#define SA struct sockaddr
#define SERVER_PORT 5566
#define PASSIVE_PORT 20000
#define MAX_CLIENTS 50
#define BUFFER 1024
#define SOCKET_BUFFER 102400
#define FIREWALL_SETTING "sock.conf" 
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
struct SOCKS4_FORMAT
{
    unsigned char VN;
    unsigned char CD;
    unsigned char DST_PORT[ 2 ];
    unsigned char DST_IP[ 4 ];

    char *user_id;
    char *domain_name;

    SOCKS4_FORMAT():
        user_id( NULL ),
        domain_name( NULL )
    {
    /*
     * NULL
     */
    }
};
//***********************************************
typedef SOCKS4_FORMAT SOCKS4_REQUEST;
typedef SOCKS4_FORMAT SOCKS4_REPLY;
//***********************************************
#endif
