#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
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

#define SA 			struct sockaddr
#define MAXLINE     1024
#define MAXBUFFER   2000000

void echo( FILE *fp, int sockfd )
{
    /*
	char sendline[ MAXBUFFER ], receive[ MAXBUFFER ];
    while( fgets( sendline, MAXBUFFER, fp ) != NULL )
	{
		write( sockfd, sendline, strlen( sendline ) );
		bzero( sendline, MAXBUFFER )
	
	}
	*/;
}

int main( int argc, char** argv )
{
	struct sockaddr_in  serveraddr;
    int                 sockfd;
	
	if( argc != 3 )
	{
	    printf( "client <IP> <port>");
	    return 0;
	}
    
    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    bzero( &serveraddr, sizeof( serveraddr ) );
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons( atoi( argv[ 2 ] ) );
    inet_pton( AF_INET, argv[ 1 ], &serveraddr.sin_addr );

    connect( sockfd, ( SA *) &serveraddr, sizeof(serveraddr) );
    
    //echo( stdin, sockfd );
    
    return 0;
}



