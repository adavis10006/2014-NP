#ifndef __SAFETY_FUNCTION__
#define __SAFETY_FUNCTION__
//***********************************************
#include <stdarg.h>
#include "socket_header.h"
//***********************************************
void sig_chld_server(int signo);
void sig_chld_cgi(int signo);
//***********************************************
size_t safety_readline(int fd, char *vptr, size_t max_len);
//***********************************************
void server_handle(int socket_fd, const char *debug_string, ...);
void debug_handle(const char *debug_string, ...);
void error_handle(const char *error_string, bool handle_exit = false, ...);
//***********************************************
#endif
