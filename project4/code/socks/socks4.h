#ifndef __SOCKS4__
#define __SOCKS4__
//***********************************************
#include "firewall.h"
//***********************************************
class SOCKS4
{
public:
	SOCKS4( int source_socket, int passive_port, FIREWALL *firewall, struct sockaddr_in source_addr );	
private:
	void LISTEN();
	void REDIRECT( int destination_socket );

	int source_socket;
    unsigned short passive_port;
	FIREWALL *firewall;
    struct sockaddr_in source_addr;
};
//***********************************************
#endif
