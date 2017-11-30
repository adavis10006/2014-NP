#include "multiclient.h"
//***********************************************
const char *header = 
"Content-type: text/html; charset=utf-8\r\n"
"Cache-Control: max-age=0\r\n"
"\r\n";
const char *template_body_head =
"<html>\n"
"<head>\n"
"\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n"
"\t<title>Network Programming Homework 3</title>\n"
"</head>\n"
"<body bgcolor=#336699>\n"
"\t<font face=\"Courier New\" size=2 color=#FFFF99>\n";
const char *template_table_head =
"\t<table width=\"800\" border=\"1\">\n"
"\t\t<tr>\n"
"\t\t<!--td>140.113.210.145</td><td>140.113.210.145</td><td>140.113.210.145</td><td>140.113.210.145</td><td>140.113.210.145</td></tr-->\n";
const char *template_table_body =
"\t\t<tr>\n"
"\t\t<td valign=\"top\"><pre id=\"m0\"></pre></td><td valign=\"top\"><pre id=\"m1\"></pre></td><td valign=\"top\"><pre id=\"m2\"></pre></td><td valign=\"top\"><pre id=\"m3\"></pre></td><td valign=\"top\"><pre id=\"m4\"></pre></td></tr>\n"
"\t</table>\n";
const char *template_body_end =
"\t</font>\n"
"</body>\n"
"</html>\n";
//***********************************************
void get_parameter( char **query_from, ENVIRONMENT_LIST *parameter )
{
    char *seperate_pointer = strchr( *query_from, '=' ), *end_pointer = strchr( *query_from, '&' );

    if( seperate_pointer == NULL || ( end_pointer != NULL && end_pointer - seperate_pointer < 0  ) )
    {
        *query_from = NULL;

        error_handle( "parameter: parameter is not legal\n", true );
    }

    if( end_pointer == NULL )
        end_pointer = *query_from + strlen( *query_from );

    char name[ BUFFER + 1 ], value[ BUFFER + 1 ];

    memset( name, 0, sizeof( name ) );
    memset( value, 0, sizeof( value ) );

    strncpy( name, *query_from, seperate_pointer - *query_from );
    strncpy( value, seperate_pointer + 1, end_pointer - ( seperate_pointer + 1 ) );

    parameter->set_environment( name, value );

    *query_from = end_pointer == *query_from + strlen( *query_from ) ? NULL : end_pointer + 1;
}
//***********************************************
MULTICLIENT::MULTICLIENT( const char *query_string )
{
    web_output( "%s", header );

    parse_query( query_string );

    multiclient();
}
//***********************************************
void MULTICLIENT::parse_query( const char *query_string )
{
    char *host = NULL, *port = NULL, *batch = NULL, *socks_host = NULL, *socks_port = NULL, *query_from = ( char* ) query_string;

    while( query_from != NULL )
        get_parameter( &query_from, &parameter );

    parameter.debug_show();
    
    for( int i = 0; i < 5; i++ )
    {
        char host_buffer[ BUFFER + 1 ], port_buffer[ BUFFER + 1 ], batch_buffer[ BUFFER + 1 ], socks_host_buffer[ BUFFER + 1 ], socks_port_buffer[ BUFFER + 1 ];
       
        memset( host_buffer, 0, sizeof( host_buffer ) );
        memset( port_buffer, 0, sizeof( port_buffer ) );
        memset( batch_buffer, 0, sizeof( batch_buffer ) );
        memset( socks_host_buffer, 0, sizeof( socks_host_buffer ) );
        memset( socks_port_buffer, 0, sizeof( socks_port_buffer ) );

        sprintf( host_buffer, "h%d", i + 1 );
        sprintf( port_buffer, "p%d", i + 1 );
        sprintf( batch_buffer, "f%d", i + 1 );
        sprintf( socks_host_buffer, "sh%d", i + 1 );
        sprintf( socks_port_buffer, "sp%d", i + 1 );

        host = parameter.get_environment( host_buffer );
        port = parameter.get_environment( port_buffer );
        batch = parameter.get_environment( batch_buffer );
        socks_host = parameter.get_environment( socks_host_buffer );
        socks_port = parameter.get_environment( socks_port_buffer );

        if( host == NULL || port == NULL || batch == NULL, socks_host == NULL || socks_port == NULL )
            error_handle( "parse_query no result\n", true );

        client_info.push_back( new CLIENT( host, atoi( port ), batch, socks_host, atoi( socks_port ) ) );
    }

    debug_client_info();
}
//***********************************************
void MULTICLIENT::multiclient()
{
    web_output( "%s", template_body_head );
    web_output( "%s", template_table_head );

    web_output( "\t\t" );
    for( int i = 0; i < 5; i++ )
    {
        char buffer[ BUFFER + 1 ];

        memset( buffer, 0, sizeof( buffer ) );

        if( client_info[ i ]->client_state == CLIENT_CLOSE )
            sprintf( buffer, "<td>%s</td>", client_info[ i ]->host );
        else
            sprintf( buffer, "<td>no client</td>" );

        web_output( "%s", buffer );
    }
    web_output( "\n" );   

    web_output( "%s", template_table_body );

    connection();

    web_output( "%s", template_body_end );
}
//***********************************************
void MULTICLIENT::connection()
{
    int max_fd = -1;
    fd_set read_fd, write_fd;

    for( int i = 0; i < client_info.size(); i++ )
        if( client_info[ i ]->client_state == CLIENT_CLOSE )
        {
            if( client_info[ i ]->socks_exist )
            {
                debug_handle( "%d: SOCKS\n", i );

                client_info[ i ]->socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
                
                memset( &( client_info[ i ]->client_sockaddr_in ), 0, sizeof( client_info[ i ]->client_sockaddr_in ) );
                
                client_info[ i ]->host_info = gethostbyname( client_info[ i ]->host );
                client_info[ i ]->socks_host_info = gethostbyname( client_info[ i ]->socks_host );
                
                if( client_info[ i ]->host_info == NULL || client_info[ i ]->socks_host_info == NULL  )
                {
                    error_handle( "gethostbyname: host name cannot be resolved\n" );
                    client_info[ i ]->client_state = CLIENT_NOT_EXIST;
                    continue;
                }
               
                client_info[ i ]->client_sockaddr_in.sin_family = AF_INET;
                client_info[ i ]->client_sockaddr_in.sin_port = htons( client_info[ i ]->socks_port );
                memcpy( &( client_info[ i ]->client_sockaddr_in.sin_addr ), client_info[ i ]->socks_host_info->h_addr, sizeof( client_info[ i ]->socks_host_info->h_addr ) );
            }
            else
            {
                debug_handle( "%d: NORMAL\n", i );

                client_info[ i ]->socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
                
                memset( &( client_info[ i ]->client_sockaddr_in ), 0, sizeof( client_info[ i ]->client_sockaddr_in ) );
                
                client_info[ i ]->host_info = gethostbyname( client_info[ i ]->host );
                
                if( client_info[ i ]->host_info == NULL )
                {
                    error_handle( "gethostbyname: host name cannot be resolved\n" );
                    client_info[ i ]->client_state = CLIENT_NOT_EXIST;
                    continue;
                }
               
                client_info[ i ]->client_sockaddr_in.sin_family = AF_INET;
                client_info[ i ]->client_sockaddr_in.sin_port = htons( client_info[ i ]->port );
                memcpy(  &( client_info[ i ]->client_sockaddr_in.sin_addr ), client_info[ i ]->host_info->h_addr, sizeof( client_info[ i ]->host_info->h_addr ) );
            }
            
            int flag = fcntl( client_info[ i ]->socket_fd, F_GETFL, 0 );
            fcntl( client_info[ i ]->socket_fd, F_SETFL, flag | O_NONBLOCK );

            if( ( client_info[ i ]->file_pointer = fopen( client_info[ i ]->batch, "r" ) ) == NULL )
            {
                error_handle( "batch file not exist\n", false );
                client_info[ i ]->client_state = CLIENT_NOT_EXIST;
                continue;
            }

            if( connect( client_info[ i ]->socket_fd, ( struct sockaddr* ) &( client_info[ i ]->client_sockaddr_in ), sizeof( client_info[ i ]->client_sockaddr_in ) ) <= 0  )
            {
                if( errno != EINPROGRESS )
                {
                    error_handle( "connect: cannot connect to client\n" );
                    client_info[ i ]->client_state = CLIENT_NOT_EXIST;
                    continue;
                }
                
                if( client_info[ i ]->socks_exist )
                   	client_info[ i ]->client_state = CLIENT_SOCK_REQUEST_INITIAL;
				else
					client_info[ i ]->client_state = CLIENT_INITIAL;
            }   
            else
            {
                error_handle( "connect: finish connect before do select\n" );

                client_info[ i ]->client_state = CLIENT_WAIT_READ;
            }

            max_fd = client_info[ i ]->socket_fd > max_fd ? client_info[ i ]->socket_fd : max_fd; 
        }

    while( true )
    {
        bool all_client_finish = true;

        FD_ZERO( &read_fd );
        FD_ZERO( &write_fd );

        for( int i = 0; i < client_info.size(); i++ )
            if( client_info[ i ]->client_state == CLIENT_SOCK_REQUEST_INITIAL )
			{
				FD_SET( client_info[ i ]->socket_fd, &write_fd );
				all_client_finish = false;
			}
			else if( client_info[ i ]->client_state == CLIENT_SOCK_REPLY_INITIAL )
			{
				FD_SET( client_info[ i ]->socket_fd, &read_fd );
				all_client_finish = false;
			}
			else if( client_info[ i ]->client_state == CLIENT_INITIAL )
            {    
                FD_SET( client_info[ i ]->socket_fd, &read_fd );
                all_client_finish = false;
            }
            else if( client_info[ i ]->client_state == CLIENT_WAIT_READ )
            {    
                FD_SET( client_info[ i ]->socket_fd, &read_fd );
                all_client_finish = false;
            }
            else if( client_info[ i ]->client_state == CLIENT_WAIT_WRITE )
            {
                FD_SET( client_info[ i ]->socket_fd, &write_fd );
                all_client_finish = false;
            }

        if( all_client_finish )
            break;

        int ready_client_amount;

        if( ( ready_client_amount = select( max_fd + 1, &read_fd, &write_fd, NULL, NULL ) ) < 0 )
            if( errno != EINPROGRESS )
                continue;

        for( int i = 0; i < client_info.size(); i++ )
        {
            if( client_info[ i ]->client_state == CLIENT_NOT_EXIST )
                continue;
            if( client_info[ i ]->client_state == CLIENT_CLOSE )
            {
                error_handle( "client: client not handle initial" );

                continue;
            }

            int socket_fd = client_info[ i ]->socket_fd;

			if( FD_ISSET( socket_fd, &write_fd ) && client_info[ i ]->client_state == CLIENT_SOCK_REQUEST_INITIAL )
			{
				int error_type;
                unsigned int error_len = sizeof( error_type );

                if( getsockopt( socket_fd, SOL_SOCKET, SO_ERROR, ( void* ) &error_type, &error_len ) < 0 || error_type != 0 )// test if not connect
                {
                    error_handle( "connect: connection has error happened\n" );
					close( socket_fd );
                    client_info[ i ]->client_state = CLIENT_NOT_EXIST;
                }
                else
                {                 
					char request_buffer[ BUFFER + 1 ];
                    int sock_ip = inet_addr( client_info[ i ]->host_info->h_addr );

                    memset( request_buffer, 0, sizeof( request_buffer ) );

                    request_buffer[ 0 ] = 0x04;
                    request_buffer[ 1 ] = 0x01;
                    request_buffer[ 2 ] = ( client_info[ i ]->port >> 8 ) & 0xFF;
                    request_buffer[ 3 ] = client_info[ i ]->port & 0xFF;
                    request_buffer[ 4 ] = client_info[ i ]->host_info->h_addr[ 0 ];
                    request_buffer[ 5 ] = client_info[ i ]->host_info->h_addr[ 1 ];
                    request_buffer[ 6 ] = client_info[ i ]->host_info->h_addr[ 2 ];
                    request_buffer[ 7 ] = client_info[ i ]->host_info->h_addr[ 3 ];
                    memcpy( request_buffer + 8, "CGI", sizeof( "CGI" ) );
                    
					write( client_info[ i ]->socket_fd, request_buffer, sizeof( request_buffer ) ); 
               
					client_info[ i ]->client_state = CLIENT_SOCK_REPLY_INITIAL;

					debug_handle("[%d] in request\n", i);
            	}

				if( --ready_client_amount <= 0 )
                	break;
			}

			if( FD_ISSET( socket_fd, &read_fd ) && client_info[ i ]->client_state == CLIENT_SOCK_REPLY_INITIAL )
			{
				char reply_buffer[ BUFFER + 1 ];
				int read_len;
                    
				memset( reply_buffer, 0, sizeof( reply_buffer ) );

				if( ( read_len = read( client_info[ i ]->socket_fd, reply_buffer, 10 ) ) > 0 && reply_buffer[ 1 ] == 0x5A )
					client_info[ i ]->client_state = CLIENT_INITIAL;
				else
				{
					error_handle( "SOCKS no reply or reject\n" );
					close( client_info[ i ]->socket_fd );
					client_info[ i ]->client_state = CLIENT_NOT_EXIST;	
				}

				debug_handle( "[%d] in reply\n", i );
				
				if( --ready_client_amount <= 0 )
					break;
			}

            if( FD_ISSET( socket_fd, &read_fd ) && client_info[ i ]->client_state == CLIENT_INITIAL )
            {
				debug_handle( "[%d] in initial\n", i );
                int error_type;
                unsigned int error_len = sizeof( error_type );

                if( !client_info[ i ]->socks_exist && ( getsockopt( socket_fd, SOL_SOCKET, SO_ERROR, ( void* ) &error_type, &error_len ) < 0 || error_type != 0 ) )// test if not connect
                {
                    error_handle( "connect: connection has error happened\n" );
					close( socket_fd );
                    client_info[ i ]->client_state = CLIENT_NOT_EXIST;
                }
                else
                {                 
                    client_info[ i ]->client_state = CLIENT_WAIT_READ;

                    if( --ready_client_amount <= 0 )
                        break;
                }
            }

            if( FD_ISSET( socket_fd, &read_fd ) && client_info[ i ]->client_state == CLIENT_WAIT_READ )
            {
                int read_len;
                char read_buffer[ SOCKET_BUFFER + 1 ];

                memset( read_buffer, 0, sizeof( read_buffer ) );
                
                if( ( read_len = safety_readline( socket_fd, read_buffer, SOCKET_BUFFER ) ) > 0 )
                {
                    web_script( i, read_buffer, read_len );

                    if( strncmp( read_buffer, "% ", 2 ) == 0 )
                        client_info[ i ]->client_state = CLIENT_WAIT_WRITE;
                }
                else if( read_len == 0 )
                {
                    close( socket_fd );
                    client_info[ i ]->client_state = CLIENT_NOT_EXIST;
                }

                if( --ready_client_amount <= 0 )
                    break;
            }

            if( FD_ISSET( socket_fd, &write_fd ) && client_info[ i ]->client_state == CLIENT_WAIT_WRITE )
            {
                int read_len, write_len;

                int read_limit = &( client_info[ i ]->write_buffer[ SOCKET_BUFFER ] ) - client_info[ i ]->write_output;

                if( client_info[ i ]->write_input == client_info[ i ]->write_output )
                {
                    char read_buffer[ SOCKET_BUFFER + 1 ];
                    memset( read_buffer, 0, sizeof( read_buffer ) );
                
                    if( fgets( read_buffer, read_limit, client_info[ i ]->file_pointer ) != NULL )
					{
                    	debug_handle( "[ %d ] %s", i + 1, read_buffer );

                    	read_len = strlen( read_buffer );
                    	strcpy( client_info[ i ]->write_output, read_buffer );
					}
					else
						return;
                }   
                else
                    read_len = 0;

                web_script( i, client_info[ i ]->write_output, read_len );

                client_info[ i ]->write_output += read_len;

                int write_limit = client_info[ i ]->write_output - client_info[ i ]->write_input;

                if( ( write_len = safety_write( socket_fd, client_info[ i ]->write_input, write_limit ) ) >= 0 ) 
                    if( write_len == write_limit )
                    {    
                        client_info[ i ]->write_input = client_info[ i ]->write_output = client_info[ i ]->write_buffer;
                        client_info[ i ]->client_state = CLIENT_WAIT_READ;
                    }
                    else
                        client_info[ i ]->write_input += write_len;

                if( --ready_client_amount <= 0 )
                    break;
            }
        }  
    }
}
//***********************************************
void MULTICLIENT::debug_client_info()
{
#if defined( DEBUG_CLIENT )
    fprintf( stderr, "client_info length: %lu\n", client_info.size() );
    for( int i = 0; i < client_info.size(); i++ )
    {
        fprintf( stderr, "number %d:\n", i );
        fprintf( stderr, "\thost: '%s'\n", client_info[ i ]->host );
        fprintf( stderr, "\tport: '%d'\n", client_info[ i ]->port );
        fprintf( stderr, "\tbatch: '%s'\n", client_info[ i ]->batch );
        fprintf( stderr, "\tsocks_host: '%s'\n", client_info[ i ]->socks_host );
        fprintf( stderr, "\tsocks_port: '%d'\n", client_info[ i ]->socks_port );
    }  
#endif   
}
//***********************************************
