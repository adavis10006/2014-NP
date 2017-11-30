#ifndef __MULTICLIENT__
#define __MULTICLIENT__
//***********************************************
#define DEBUG_CLIENT
//***********************************************
#include "environment_list.h"
#include <deque>
//***********************************************
struct CLIENT 
{
    char *host, *batch, *socks_host;
    int port, socks_port;
    bool socks_exist;
    bool exist;
    
    struct hostent *host_info, *socks_host_info;

    int socket_fd;
    struct sockaddr_in client_sockaddr_in;

    FILE *file_pointer; 
    int file_fd;
   
    CLIENT_STATE client_state;
    
    char write_buffer[ SOCKET_BUFFER + 1 ];
    char *write_input, *write_output;
    
    CLIENT( const char *input_host, const int input_port, const char *input_batch, const char *input_socks_host, const int input_socks_port ): 
        port( input_port ),
        socks_port( input_socks_port )
    {
        host = new char[ strlen( input_host ) + 1 ];
        batch = new char[ strlen( input_batch ) + 1 ];
        socks_host = new char[ strlen( input_socks_host ) + 1 ];

        memset( host, 0, sizeof( host ) );
        memset( batch, 0, sizeof( batch ) );
        memset( socks_host, 0, sizeof( socks_host ) );

        strcpy( host, input_host );
        strcpy( batch, input_batch );
        strcpy( socks_host, input_socks_host );

        if( strlen( input_host ) != 0 )
            client_state = CLIENT_CLOSE;
        else 
            client_state = CLIENT_NOT_EXIST;
      
        if( strlen( input_socks_host ) != 0 )
            socks_exist = true;
        else
            socks_exist = false;

        write_input = write_output = write_buffer;
    }
};
//***********************************************
class MULTICLIENT
{
public:
    MULTICLIENT( const char *query_string );
    
    void parse_query( const char *query_string );
    
    void multiclient();
    void connection();

    void debug_client_info();
private:
    ENVIRONMENT_LIST parameter;

    std::deque< CLIENT * > client_info;    
};
//***********************************************
#endif
