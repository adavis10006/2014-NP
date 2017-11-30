#ifndef __COMMUNICATION__
#define __COMMUNICATION__
//***********************************************
#include <stdarg.h>
#include "socket_header.h"
//***********************************************
size_t safety_readline(int fd, char *vptr, size_t max_len);
size_t safety_write(int fd, char *vptr, size_t max_len);
//***********************************************
void web_output(const char *output_string, ...);
//***********************************************
void web_script(int client_id, const char *output_pointer, int output_len);
//***********************************************
void debug_handle(const char *debug_string, ...);
//***********************************************
void error_handle(const char *error_string, bool handle_error = false, ...);
//***********************************************
#endif
