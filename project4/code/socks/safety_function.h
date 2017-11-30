#ifndef __SAFETY_FUNCTION__
#define __SAFETY_FUNCTION__
//***********************************************
#include <stdarg.h>
#include "socket_header.h"
//***********************************************
int safety_port( int port );
//***********************************************
void sig_chld_server( int signo );
//***********************************************
int passiveTCP( const char *hostname, const char *service, int qlen );
int connectTCP( const char *hostname, const char *service );
//***********************************************
void va_handle(const char *format_string, va_list args, char *result_string );
void server_handle( int socket_fd, const char *debug_string, ... );
void debug_handle( const char *debug_string, ... );
void error_handle( const char *error_string, bool handle_exit = false, ... );
//***********************************************
#endif
