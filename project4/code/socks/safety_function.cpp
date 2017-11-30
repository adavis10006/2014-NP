#include "safety_function.h"
//***********************************************
int safety_port( int port )
{
    if( port < 1024 || port > 65535 )
        return SERVER_PORT;
    return port;
}
//***********************************************
void sig_chld_server( int signo )
{
    pid_t pid;
    int stat;

    while( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 );
}
//***********************************************
int connectsock( const char *hostname, const char *service, const char *transport );
int passivesock( const char *hostname, const char *service, const char *transport, int qlen, int flag );
//***********************************************
int passiveTCP( const char *hostname, const char *service, int qlen )
{
    return passivesock( hostname, service, "tcp", qlen, 0 );
}
//***********************************************
int passivesock( const char *hostname, const char *port, const char *transport, int qlen, int flag )
{
    struct sockaddr_in sin; /* an Internet endpoint address */
    int s, type, reuse; /* socket descriptor and socket type */

    memset( &sin, 0, sizeof( sin ) );
    sin.sin_family = AF_INET;

    if( hostname == NULL ) 
        sin.sin_addr.s_addr = htonl( INADDR_ANY );
    else 
        sin.sin_addr.s_addr = inet_addr( hostname );

    /* Map service name to port number */
    if( ( sin.sin_port = htons( ( unsigned short ) atoi( port ) ) ) == 0 )
        return -1;

    /* Use protocol to choose a socket type */
    if( strcmp( transport, "udp" ) == 0 )
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

    /* Allocate a socket */
    s = socket( AF_INET, type, 0 );
    if( s < 0 )
        return -1;

    /* REUSEADDR */
    reuse = 1;
    if( flag == 1 )
        if( setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( int ) ) < 0 )
            return -1;

    /* Bind the socket */
    if( bind( s, ( struct sockaddr * ) &sin, sizeof( sin ) ) < 0 )
        return -1;
    if( type == SOCK_STREAM && listen( s, qlen ) < 0 )
        return -1;

    return s;
}
//***********************************************
int connectTCP( const char *host, const char *service )
{
    return connectsock( host, service, "tcp" );
}
//***********************************************
int connectsock( const char *host, const char *service, const char *transport )
{
    struct hostent *phe; /* pointer to host information entry */
    struct servent *pse; /* pointer to service information entry */
    struct protoent *ppe; /* pointer to protocol information entry */
    struct sockaddr_in sin; /* an Internet endpoint address */
    int s, type; /* socket descriptor and socket type */

    memset( &sin, 0, sizeof( sin ) );
    sin.sin_family = AF_INET;

    /* Map service name to port number */
    if( pse = getservbyname( service, transport ) )
        sin.sin_port = pse->s_port;
    else if( ( sin.sin_port = htons( ( unsigned short ) atoi( service ) ) ) == 0 )
        return -1;

    /* Map host name to IP address, allowing for dotted decimal */
    if( phe = gethostbyname( host ) )
        memcpy( &sin.sin_addr, phe->h_addr, phe->h_length );
    else if( ( sin.sin_addr.s_addr = inet_addr( host ) ) == INADDR_NONE )
        return -1;                  
    
    /* Map transport protocol name to protocol number */
    if( ( ppe = getprotobyname( transport ) ) == 0 )
        return -1;
    
    /* Use protocol to choose a socket type */
    if( strcmp( transport, "udp" ) == 0 )
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    
    /* Allocate a socket */
    s = socket( AF_INET, type, ppe->p_proto );
    if( s < 0 )
        return -1;
    
    /* Connect the socket */
    if( connect( s, ( struct sockaddr * ) &sin, sizeof( sin ) ) < 0 )
        return -1;

    return s;
}
//***********************************************
void va_handle( const char *format_string, va_list args, char *result_string )
{
    memset( result_string, 0, sizeof( result_string ) );
    
    vsprintf( result_string, format_string, args );
    fputs( result_string, stderr );
    fflush( stderr );
}
//***********************************************
void server_handle( int socket_fd, const char *server_string, ... )
{
    char socket_buffer[ SOCKET_BUFFER + 1 ];
    va_list args;
    
    va_start( args, server_string );
    va_handle( server_string, args, socket_buffer );
    va_end( args );

    if( write( socket_fd, socket_buffer, strlen( socket_buffer ) ) < 0 )
        if( errno != EWOULDBLOCK || errno != EINTR )
            error_handle( "write: write to socket error\n" );
}
//***********************************************
void debug_handle( const char *debug_string, ... )
{
    char socket_buffer[ SOCKET_BUFFER + 1 ];
    va_list args;

    va_start( args, debug_string );
    va_handle( debug_string, args, socket_buffer );
    va_end( args );
}
//***********************************************
void error_handle( const char *error_string, bool handle_error, ... )
{
    char socket_buffer[ SOCKET_BUFFER + 1 ];
    va_list args;

    va_start( args, handle_error );
    va_handle( error_string, args, socket_buffer );
    va_end( args );
    
    if( handle_error )
    {
        fprintf( stderr, "***NOW HAVE TO SHUT DOWN SERVER***\n" );
        fflush( stderr );
        exit( 0 );
    }
}
//***********************************************
