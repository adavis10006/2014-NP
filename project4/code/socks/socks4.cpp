#include "socks4.h"
//***********************************************
bool mask_ip( SOCKS4_REQUEST *request )
{
    if( request->DST_IP[ 0 ] == 0 && request->DST_IP[ 1 ] == 0 && request->DST_IP[ 2 ] == 0 )
        return true;
    return false;
}
//***********************************************
bool parse_request( int source_socket, SOCKS4_REQUEST *request )
{
    char socket_buffer[ SOCKET_BUFFER + 1 ];
    int read_length;
    
    if( ( read_length = read( source_socket, &( request->VN ), sizeof( request->VN ) ) ) != 1 )
    {   
        error_handle( "READ REQUEST VERSION ERROR\n" );
        return false;
    }

    if( ( read_length = read( source_socket, &( request->CD ), sizeof( request->CD ) ) ) != 1 )
    {
        error_handle( "READ REQUEST COMMAND ERROR\n" );
        return false;
    }
    
    if( ( read_length = read( source_socket, request->DST_PORT, sizeof( request->DST_PORT ) ) ) != 2 )
    {
        error_handle( "READ REQUEST PORT ERROR\n" );
        return false;
    }

    if( ( read_length = read( source_socket, request->DST_IP, sizeof( request->DST_IP ) ) ) != 4 )
    {
        error_handle( "READ REQUEST IP ERROR\n" );
        return false;
    }

    memset( socket_buffer, 0, sizeof( socket_buffer ) );
    if( ( read_length = read( source_socket, socket_buffer, sizeof( socket_buffer ) ) ) <= 0 )
    {
        error_handle( "READ REQUEST USER ID ERROR\n" );
        return false;
    }
    else if( mask_ip( request ) )
	{
			for( int i = 0; i < read_length; i++ )
				if( socket_buffer[ i ] == '\0' )
				{
       				strcpy( request->user_id, socket_buffer );
					strcpy( request->domain_name, socket_buffer + i + 1 );
					break;		
				}
	}
	else
		strcpy( request->user_id, socket_buffer );				
    return true;
}
//***********************************************
void generate_reply( int source_socket, SOCKS4_REPLY reply, bool access_status  = true )
{
    char reply_buffer[ 9 ];
   
    memset( reply_buffer, 0, sizeof( reply_buffer ) );
    reply_buffer[ 1 ] = access_status ? 0x5A : 0x5B; 
    memcpy( reply_buffer + 2, reply.DST_PORT, sizeof( reply.DST_PORT ) );
    memcpy( reply_buffer + 4, reply.DST_IP, sizeof( reply.DST_IP ) );

    write( source_socket, reply_buffer, 8 );
}
//***********************************************
SOCKS4::SOCKS4( int source_socket, int passive_port, FIREWALL *firewall, struct sockaddr_in source_addr ):
    source_socket( source_socket ), 
    passive_port( passive_port + PASSIVE_PORT ),
    firewall( firewall ),
    source_addr( source_addr )
{
    debug_handle( "NEW REQUEST IN PID: %d\n", getpid() );
    LISTEN();
    debug_handle( "FINISH REQUEST PID: %d\n", getpid() );
}
//***********************************************
void SOCKS4::LISTEN()
{
    SOCKS4_REQUEST request;
    SOCKS4_REPLY reply;

    memset( &request, 0, sizeof( request ) );
    memset( &reply, 0, sizeof( reply ) );

    if( !parse_request( source_socket, &request ) )
        return;

    reply = request;
    
    char source_ip[ 16 ], source_port[ 6 ], destination_ip[ 16 ], destination_port[ 6 ];

    memset( source_ip, 0, sizeof( source_ip ) );
    memset( source_port, 0, sizeof( source_port ) );
    memset( destination_ip, 0, sizeof( destination_ip ) );
    memset( destination_port, 0, sizeof( destination_port ) );
    
    inet_ntop( AF_INET, &source_addr.sin_addr, source_ip, sizeof( source_ip ) );
    sprintf( source_port, "%u", ntohs( source_addr.sin_port ) );
    sprintf( destination_ip, "%u.%u.%u.%u", request.DST_IP[ 0 ], request.DST_IP[ 1 ], request.DST_IP[ 2 ], request.DST_IP[ 3 ] );
    sprintf( destination_port, "%u", ( request.DST_PORT[ 0 ] << 8 ) + request.DST_PORT[ 1 ] );

    debug_handle( "VN: %d CD: %d SRC IP: %s PROT: %s DST IP: %s PORT: %s ID: %s DOMAIN: %s\n", request.VN, request.CD, source_ip, source_port, destination_ip, destination_port, request.user_id, request.domain_name == NULL ? "(NULL)": request.domain_name );
    
    if( firewall->check( source_ip, source_port, destination_ip, destination_port, request.CD ) )
    {
        int redirect_socket, passive_socket;

        switch( request.CD )
        {
            case 0x01:
                debug_handle( "SOCKS_CONNECT GRANTED ...\n" );

                if( ( redirect_socket = connectTCP( ( const char * ) destination_ip, ( const char * ) destination_port ) ) < 0 )
                {
                    error_handle( "CONNECT SOCKET OPEN FAIL\n" );
                    return;
                }

                reply.VN = 0x00;
                reply.CD = redirect_socket > -1 ? 0x5A : 0x5B;
                memcpy( reply.DST_PORT, request.DST_PORT, sizeof( reply.DST_PORT ) );
                memcpy( reply.DST_IP, request.DST_IP, sizeof( reply.DST_IP ) );

                generate_reply( source_socket, reply );
                break;
            case 0x02:
                socklen_t client_len;
                struct sockaddr_in client_addr;
                char passive_port_buffer[ 2 ];
                
                debug_handle( "SOCKS_BIND GRANTED ...\n" );

                sprintf( passive_port_buffer,  "%u", passive_port );
                if( ( passive_socket = passiveTCP( NULL, passive_port_buffer, MAX_CLIENTS ) ) < 0 )
                {
                    error_handle( "BIND SOCKET OPEN FAIL\n" );
                    return;
                }

                reply.VN = 0x00;
                reply.CD = passive_socket > -1 ? 0x5A : 0x5B;
                reply.DST_PORT[ 0 ] = passive_port >> 8 & 0xFF;
                reply.DST_PORT[ 1 ] = passive_port & 0xFF;
                memset( reply.DST_IP, 0, sizeof( reply.DST_IP ) );

                generate_reply( source_socket, reply );
                
                redirect_socket = accept( passive_socket, ( SA* ) &client_addr, &client_len );
                
                generate_reply( source_socket, reply );
                break;
            default:
                error_handle( "[ERROR] SOCKS4_REUEST REJECT.\n", false );
        }

        REDIRECT( redirect_socket );
    }
    else
    {
        switch( request.CD )
        {
            case 0x01:
                debug_handle( "SOCKS4_CONNECT REJECT.\n" );
                break;
            case 0x02:
                debug_handle( "SOCKS4_BIND REJECT.\n" );
                break;
            default:
                error_handle( "[ERROR] SOCKS4_REUEST REJECT.\n", false );
        }
        generate_reply( source_socket, reply, false );
    }
}
//***********************************************
void SOCKS4::REDIRECT( int destination_socket )
{
    int max_fd, read_length, write_length, nready, buffer_length;
    char SRC2DST_buffer[ SOCKET_BUFFER + 1 ], DST2SRC_buffer[ SOCKET_BUFFER + 1 ];
    char *SRC2DST_read_ptr, *SRC2DST_write_ptr, *DST2SRC_read_ptr, *DST2SRC_write_ptr;
    int SRC2DST_write_limit, DST2SRC_write_limit;
    bool SRC2DST_finish = false, DST2SRC_finish = false;
    char buffer[ 20 + 1 ];
    fd_set read_fds, write_fds;

    SRC2DST_read_ptr = SRC2DST_write_ptr = SRC2DST_buffer;
    DST2SRC_read_ptr = DST2SRC_write_ptr = DST2SRC_buffer;
    max_fd = source_socket > destination_socket ? source_socket : destination_socket;

    while( true )
    {
        FD_ZERO( &read_fds );
        FD_ZERO( &write_fds );

        SRC2DST_write_limit = SRC2DST_read_ptr - SRC2DST_write_ptr;
        DST2SRC_write_limit = DST2SRC_read_ptr - DST2SRC_write_ptr;
        
        bool set_fds = false;
        if( !SRC2DST_finish && SRC2DST_read_ptr == SRC2DST_buffer )
        {
            FD_SET( source_socket, &read_fds );
            set_fds = true;
        }
        if( !DST2SRC_finish && DST2SRC_read_ptr == DST2SRC_buffer )
        {    
            FD_SET( destination_socket, &read_fds );
            set_fds = true;
        }
        if( DST2SRC_write_limit != 0 )
        {    
            FD_SET( source_socket, &write_fds );
            set_fds = true;
        }
        if( SRC2DST_write_limit != 0 )
        {    
            FD_SET( destination_socket, &write_fds );
            set_fds = true;
        }

        if ( !set_fds ) 
        {
			debug_handle("done type1\n");
            close( source_socket );
			close( destination_socket );
			shutdown( source_socket, SHUT_WR );
            shutdown( destination_socket, SHUT_WR );
            return;
        }

        if( ( nready = select( max_fd + 1, &read_fds, &write_fds, NULL, NULL ) ) < 0 ) 
            error_handle( "SELECT ERROR\n" );

        if( FD_ISSET( source_socket, &read_fds ) )
            if( ( read_length = read( source_socket, SRC2DST_read_ptr, SOCKET_BUFFER ) ) > 0 )
                SRC2DST_read_ptr += read_length;
            else if( read_length == 0 )
			{
				SRC2DST_finish = true;
        		debug_handle("done type3 pid: %d\n", getpid() );
            	close( source_socket );
				close( destination_socket );
				shutdown( source_socket, SHUT_WR );
            	shutdown( destination_socket, SHUT_WR );
            	return;
        	}

        if( FD_ISSET( destination_socket, &read_fds ) )
            if( ( read_length = read( destination_socket, DST2SRC_read_ptr, SOCKET_BUFFER ) ) > 0 )
                DST2SRC_read_ptr += read_length;
            else if( read_length == 0 )
			{	
				DST2SRC_finish = true;
				debug_handle("done type3 pid: %d\n", getpid() );
            	close( source_socket );
				close( destination_socket );
				shutdown( source_socket, SHUT_WR );
            	shutdown( destination_socket, SHUT_WR );
            	return;
        	}

        if( FD_ISSET( source_socket, &write_fds ) )
        {
            if( ( write_length = write( source_socket, DST2SRC_write_ptr, DST2SRC_write_limit ) ) > 0 )
            {    
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = write_length >= 20 ? 20 : write_length;
                strncpy( buffer, DST2SRC_write_ptr, buffer_length );
                for( int i = 0; i < buffer_length; i++ )
                    if( buffer[ i ] < 32 || buffer[ i ] > 126 )
                        buffer[ i ] = ' ';
                buffer[ buffer_length ] = '\0';
                //debug_handle( "REDIRECT DST->SRC:\n%s...\n", buffer );
 
                DST2SRC_write_ptr += write_length;
            }
            
            if( DST2SRC_write_ptr == DST2SRC_read_ptr )
                DST2SRC_read_ptr = DST2SRC_write_ptr = DST2SRC_buffer;

            if( DST2SRC_finish && DST2SRC_write_ptr == DST2SRC_buffer )
            {
            	close( source_socket );
				close( destination_socket );
				shutdown( source_socket, SHUT_WR );
            	shutdown( destination_socket, SHUT_WR );
            	return;
        	}
        }

        if( FD_ISSET( destination_socket, &write_fds ) )
        {
            if( ( write_length = write( destination_socket, SRC2DST_write_ptr, SRC2DST_write_limit ) ) > 0 )
            {
                memset( buffer, 0, sizeof( buffer ) );
                buffer_length = write_length >= 20 ? 20 : write_length;
                strncpy( buffer, SRC2DST_write_ptr, buffer_length );
                for( int i = 0; i < buffer_length; i++ )
                    if( buffer[ i ] < 32 || buffer[ i ] > 126 )
                        buffer[ i ] = ' ';
                buffer[ buffer_length ] = '\0';
                //debug_handle( "REDIRECT SRC->DST:\n%s...\n", buffer );
 
                SRC2DST_write_ptr += write_length;
            }

            if( SRC2DST_write_ptr == SRC2DST_read_ptr )
                SRC2DST_read_ptr = SRC2DST_write_ptr = SRC2DST_buffer;

            if( SRC2DST_finish && SRC2DST_write_ptr == SRC2DST_buffer )
            {
            	close( source_socket );
				close( destination_socket );
				shutdown( source_socket, SHUT_WR );
            	shutdown( destination_socket, SHUT_WR );
            	return;
        	}
        }
    }
}
//***********************************************
