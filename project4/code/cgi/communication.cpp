#include "communication.h"
//***********************************************
const char *error_output_head =
"<html>\r\n"
"\t<title>ERROR</title>\r\n"
"\t<body>";
const char *error_output_end =
"</body>\r\n"
"</html>\r\n";
//***********************************************
void add_quote(std::string &string, std::string &result)
{
    result.clear();

    for(int i = 0; i < string.size(); i++)
        switch(string[i])
        {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "&quot;";
                break;
            case '&':
                result += "&amp;";
                break;
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '\t':
                result += "\\t";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\n':
                result += "\\n";
                break;
            default:
                result += string[i];
                break;
        }
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
                error_handle("read: read error happened\n", false);
            //else
                //debug_handle("read normal nonblocking check\n");

            continue;    
        }
    }

    *ptr = 0;

    return ptr - vptr;
}
//***********************************************
size_t safety_write(int fd, char *vptr, size_t max_len)
{
    size_t total_write_len = 0, write_len;

    while(total_write_len < max_len)
    {
        if((write_len = write(fd, vptr + total_write_len, max_len - total_write_len)) < 0)
        {    
            if(errno != EWOULDBLOCK)
                error_handle("write: write error happened\n", false);
            //else
                //debug_handle("write normal nonblocking check\n");
            
            continue;
        }

        total_write_len += write_len;
    }

    return total_write_len;
}
//***********************************************
void web_output(const char* output_string, ...)
{
    char socket_buffer[SOCKET_BUFFER + 1];
    va_list args;

    memset(socket_buffer, 0, sizeof(socket_buffer));

    va_start(args, output_string);
    vsprintf(socket_buffer, output_string, args);
    va_end(args);
    
    fputs(socket_buffer, stdout);debug
    fflush(stdout);
}
//***********************************************
void web_script(int client_id, const char* output_pointer, int output_len)
{
    char id_buffer[BUFFER + 1];

    memset(id_buffer, 0, sizeof(id_buffer));

    sprintf(id_buffer, "%d", client_id);

    std::string output_string(output_pointer, output_len), id_string(id_buffer), output_quote_string; 

    add_quote(output_string, output_quote_string);

    output_string = "<script>document.all['m" + id_string + "'].innerHTML += \"" + output_quote_string + "\";</script>\n";

	//debug_handle( "[%d]: %s\n", client_id, output_string.c_str() );

    web_output("%s", output_string.c_str());
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
    {
        web_output("%s%s%s", error_output_head, socket_buffer, error_output_end);

        exit(0);
    }
}
//***********************************************
