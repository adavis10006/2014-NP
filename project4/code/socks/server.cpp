#include "socks4.h"
//***********************************************
int main( int argc, char **argv )
{
    socklen_t client_len;
    int server_port;
    struct sockaddr_in server_addr, client_addr;
    int socket_fd, connect_fd;
    pid_t child_pid;
    
    if( argc == 1 )
        server_port = SERVER_PORT;
    else if( argc == 2 )
        server_port = safety_port( atoi( argv[ 1 ] ) );
    else
        error_handle( "NOT LEGAL PARAMTER\n", true );

    printf( "[SERV] SOCKS server version %s starting\n", SOCKS4_VERSION );
    
    if( ( socket_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
        error_handle( "SOCKET CREATE ERROR\n", true );

    bzero( &server_addr, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    server_addr.sin_port = htons( server_port );
    
    if( bind( socket_fd, ( SA* ) &server_addr, sizeof( server_addr ) ) < 0 )
        error_handle( "SERVER BIND ERROR\n", true );

    if( listen( socket_fd, MAX_CLIENTS ) < 0 )
        error_handle( "SERVER LISTEN ERROR\n", true );

    signal( SIGCHLD, sig_chld_server );

    FIREWALL firewall( FIREWALL_SETTING );

    int client_amount = 0;

    while( true )
    {
        client_len = sizeof( client_addr );
        connect_fd = accept( socket_fd, ( SA* ) &client_addr, &client_len );

        if( connect_fd == -1 )
        {
            //error_handle( "SERVER ACCEPT ERROR\n", false );
            continue;
        }
        
        client_amount++;

        if( ( child_pid = fork() ) == 0 )
        {
            close( socket_fd );
            SOCKS4 socks4( connect_fd, client_amount, &firewall, client_addr );
            close( connect_fd );
            return 0;
        }
        else if( child_pid > 0 )
            close( connect_fd );
        else
            error_handle( "SERVER CREATE FORK ERROR\n", true );
    }
    return 0;
}
//***********************************************
