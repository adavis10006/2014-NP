#include "firewall.h"
//***********************************************
bool parser( char *buffer, char **ip_port, char *type )
{
    int index = 0;
    
    for( char *ptr = strtok( ( char * ) buffer, " \t\r\n" ); ptr != NULL; ptr = strtok( NULL, " \t\r\n" ), index++ )
    {
        switch( index )
        {
            case 0:
                if( strcmp( ptr, "permit" ) != 0 )
                    return false;
                    break;
            case 1:
                if( strcmp( ptr, "c" ) == 0 )
                    *type = 'c'; 
                else if( strcmp( ptr, "b" ) == 0 )
                    *type = 'b';
                else
                    return false;
                    break;
            case 2:
            case 3:
            case 4:
            case 5:
                ip_port[ index - 2 ] = new char [ strlen( ptr ) + 1 ];
                strcpy( ip_port[ index - 2 ], ptr );
                break;
            default:
                return false;
        }
    }

    for( int i = 0; i < 4; i++ )
        if( ip_port[ i ] == NULL )
            return false;
    return true;        
}    
//***********************************************
FIREWALL::FIREWALL( const char *file_path )
{
    char buffer[ BUFFER + 1 ];    
    FILE *ptr = fopen( file_path, "r" );
  
    connect_whitelist.clear();
    bind_whitelist.clear();

    while( fgets( buffer, BUFFER, ptr ) != NULL )
    {
        char **ip_port = new char *[ 4 ];
        char type;

        if( parser( buffer, ip_port, &type ) )
        {
            switch( type )
            {
                case 'c':
                    debug_handle( "FIREWALL CONNECT:\n" );
                    connect_whitelist.push_back( ip_port ); 
                    break;
                case 'b':
                    debug_handle( "FIREWALL BIND:   \n" );
                    bind_whitelist.push_back( ip_port ); 
                    break;
            }
            debug_handle( "SRC IP: %s PORT: %s DST IP: %s PORT: %s\n", ip_port[ 0 ], ip_port[ 1 ], ip_port[ 2 ], ip_port[ 3 ] );
        }
    }
}
//***********************************************
FIREWALL::~FIREWALL()
{
    for( int i = 0; i < connect_whitelist.size(); i++ )
    {
        for( int j = 0; j < 4; j++ )
            delete [] connect_whitelist[ i ][ j ];
        delete [] connect_whitelist[ i ];
    }
    connect_whitelist.clear();

    for( int i = 0; i < bind_whitelist.size(); i++ )
    {
        for( int j = 0; j < 4; j++ )
            delete [] bind_whitelist[ i ][ j ];
        delete [] bind_whitelist[ i ];
    }
    bind_whitelist.clear();
}
//***********************************************
bool FIREWALL::check( char *source_ip, char *source_port, char *destination_ip, char *destination_port, char type )
{
    std::deque< char** > *whitelist;
    
    switch( type )
    {
        case 0x01: 
            whitelist = &connect_whitelist; 
            break;
        case 0x02:
            whitelist = &bind_whitelist;
			break;
		default:
			return false;
    }
    
    for( int i = 0; i < whitelist->size(); i++ )
    {
    	bool allow = true;
        
		if( strcmp( ( *whitelist )[ i ][ 0 ], "-" ) != 0 && strncmp( ( *whitelist )[ i ][ 0 ], source_ip, strlen( ( *whitelist)[ i ][ 0 ] ) ) != 0 )
        {    
            debug_handle("REJECT BY SRC IP\n");
            debug_handle( "SRC IP: %s\n", ( *whitelist )[ i ][ 0 ] );
            allow = false;
        }
        if( strcmp( ( *whitelist )[ i ][ 1 ], "-" ) != 0 && strncmp( ( *whitelist )[ i ][ 1 ], source_port, strlen( ( *whitelist)[ i ][ 1 ] ) ) != 0 )
        {    
            debug_handle("REJECT BY SRC PORT\n");
            debug_handle( "SRC PORT: %s\n", ( *whitelist )[ i ][ 1 ] );
            allow = false;
        }
        if( strcmp( ( *whitelist )[ i ][ 2 ], "-" ) != 0 && strncmp( ( *whitelist )[ i ][ 2 ], destination_ip, strlen( ( *whitelist)[ i ][ 2 ] ) ) != 0 )
        {    
            debug_handle("REJECT BY DST IP\n");
            debug_handle( "DST IP: %s\n", ( *whitelist )[ i ][ 2 ] );
            allow = false;
        }
        if( strcmp( ( *whitelist )[ i ][ 3 ], "-" ) != 0 && strncmp( ( *whitelist )[ i ][ 3 ], destination_port, strlen( ( *whitelist)[ i ][ 3 ] ) ) != 0 )
        {
            debug_handle("REJECT BY DST PORT\n");
            debug_handle( "DST PORT: %s\n", ( *whitelist )[ i ][ 3 ] );
            allow = false;
        }

        if( allow )
            return true;
    }
    return false;
}
//***********************************************
