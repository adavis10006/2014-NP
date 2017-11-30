#ifndef __FIREWALL__
#define __FIREWALL__
//***********************************************
#include "safety_function.h"
#include <deque>
//***********************************************
class FIREWALL
{
public:
    FIREWALL( const char *file_path );
    ~FIREWALL();
    bool check( char *source_ip, char *source_port, char *destination_ip, char *destination_port, char type ); 
private:
    std::deque< char ** > connect_whitelist, bind_whitelist;
};
//***********************************************
#endif
