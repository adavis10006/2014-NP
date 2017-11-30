#include "safety_function.h"
//***********************************************
void sig_chld_server(int signo)
{
    pid_t pid;
    int stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0);
}
//***********************************************   
void sig_chld_cgi(int signo)
{
    pid_t pid;
    int stat;

    while((pid = waitpid(getpid(), &stat, WNOHANG)) > 0);
}
//***********************************************
size_t safety_readline(int fd, char *vptr, size_t max_len)
{
    size_t n, rc;
    char c, *ptr;

    ptr = vptr;

    for(n = 1; n < max_len; n++)
    {
        if((rc = read(fd, &c, 1)) == 1) 
            if(c == '\n')
            {
                *ptr = '\n';
                *(ptr + 1) = 0;

                return ptr - vptr + 1;
            }
            else
                *ptr++ = c;
        else if(rc == 0) 
        {
            *ptr = 0;

            return ptr - vptr;
        } 
        else 
        {
            if(errno != EWOULDBLOCK)
                error_handle("read: read error happened\n");

            continue;
        }
    }

    *ptr = 0;

    return ptr - vptr;
}
//***********************************************
void server_handle(int socket_fd, const char *server_string, ...)
{
    char socket_buffer[SOCKET_BUFFER + 1];
    va_list args;

    memset(socket_buffer, 0, sizeof(socket_buffer));
    
    va_start(args, server_string);
    vsprintf(socket_buffer, server_string, args);
    va_end(args);

    if(write(socket_fd, socket_buffer, strlen(socket_buffer)) < 0)
        if(errno != EINTR)
            error_handle("write: write to socket error\n");
}
//***********************************************
void debug_handle(const char *debug_string, ...)
{
    char socket_buffer[SOCKET_BUFFER + 1];
    va_list args;

    memset(socket_buffer, 0, sizeof(socket_buffer));
    
    va_start(args, debug_string);
    vsprintf(socket_buffer, debug_string, args);
    va_end(args);
    
    fputs(socket_buffer, stderr);
    fflush(stderr);
}
//***********************************************
void error_handle(const char *error_string, bool handle_error, ...)
{
    char socket_buffer[SOCKET_BUFFER + 1];
    va_list args;

    memset(socket_buffer, 0, sizeof(socket_buffer));

    va_start(args, handle_error);
    vsprintf(socket_buffer, error_string, args);
    va_end(args);
    
    fputs(socket_buffer, stderr);
    fflush(stderr);
    
    if(handle_error)
        exit(0);
}
//***********************************************

