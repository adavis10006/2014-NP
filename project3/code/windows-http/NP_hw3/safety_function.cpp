#include "safety_function.h"
//***********************************************
const char *web_error_output_head =
"<html>\r\n"
"\t<title>ERROR</title>\r\n"
"\t<body>";
const char *web_error_output_end =
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
size_t safety_readline(int fd, char *vptr, size_t max_len, bool handle_nonblocking)
{
    size_t n, rc;
    char c, *ptr;

    ptr = vptr;

    for(n = 1; n < max_len; n++)
    {
        if((rc = recv(fd, &c, 1, 0)) == 1) 
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
			if((handle_nonblocking && WSAGetLastError() != WSAEWOULDBLOCK) || (!handle_nonblocking && WSAGetLastError() != WSAEINTR))
			    error_handle("read: read error happened\n");
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
	size_t total_send_len = 0, send_len;
	
	while(total_send_len < max_len) 
	{ 
		if((send_len = send(fd, vptr + total_send_len, max_len - total_send_len, 0)) < 0)
		{	
			if(WSAGetLastError() != WSAEINTR && WSAGetLastError() != WSAEWOULDBLOCK) 
				error_handle("write: write error happened\n");
			//else
				//debug_handle("write normal nonblocking check\n");

			continue;
		}

		total_send_len += send_len; 
   }

   return total_send_len;
}
//***********************************************
void web_output(int socket_fd, const char *output_string, ...)
{
	char socket_buffer[SOCKET_BUFFER + 1];
    va_list args;
	
    memset(socket_buffer, 0, sizeof(socket_buffer));
	
    va_start(args, output_string);
    vsprintf(socket_buffer, output_string, args);
    va_end(args);
	
	if(send(socket_fd, socket_buffer, strlen(socket_buffer), 0) < 0)
        if(WSAGetLastError() != WSAEINTR)
			error_handle("write: write to socket error\n");
}
//***********************************************
void web_error(int socket_fd, const char *output_pointer, ...)
{
	char socket_buffer[SOCKET_BUFFER + 1];
    va_list args;
	
    memset(socket_buffer, 0, sizeof(socket_buffer));
	
    va_start(args, output_pointer);
    vsprintf(socket_buffer, output_pointer, args);
    va_end(args);

	web_output(socket_fd, "%s%s%s", web_error_output_head, socket_buffer, web_error_output_end);
}
//***********************************************
void web_script(int socket_fd, int client_id, const char *output_pointer, int output_len)
{
	char id_buffer[BUFFER + 1];
	memset(id_buffer, 0, sizeof(id_buffer));
	
    sprintf(id_buffer, "%d", client_id);

	std::string output_string(output_pointer, output_len), id_string(id_buffer), output_quote_string; 

    add_quote(output_string, output_quote_string);
	output_string = "<script>document.all['m" + id_string + "'].innerHTML += \"" + output_quote_string + "\";</script>\n";
	
	//debug_handle("%s\n", output_quote_string.c_str());
	
	web_output(socket_fd, "%s", output_string.c_str());
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
	
	if(send(socket_fd, socket_buffer, strlen(socket_buffer), 0) < 0)
		if(WSAGetLastError() != WSAEINTR)
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
		
#ifdef _WIN32
	if(windows_env::hwnd_edit_console == NULL)
		return;
	
	SendMessage (windows_env::hwnd_edit_console, EM_SETSEL, (WPARAM) -1, (LPARAM) -1) ;
	SendMessage (windows_env::hwnd_edit_console, EM_REPLACESEL, FALSE, (LPARAM)socket_buffer);
	SendMessage (windows_env::hwnd_edit_console, EM_SCROLLCARET, 0, 0) ;
	SendMessage(windows_env::hwnd_edit_console, EM_GETLINECOUNT, 0, 0);
#else
	 fputs(socket_buffer, stderr);
     fflush(stderr);
#endif
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

#ifdef _WIN32
	if(windows_env::hwnd_edit_console == NULL)
		return;
	
	SendMessage (windows_env::hwnd_edit_console, EM_SETSEL, (WPARAM) -1, (LPARAM) -1) ;
	SendMessage (windows_env::hwnd_edit_console, EM_REPLACESEL, FALSE, (LPARAM)socket_buffer) ;
	SendMessage (windows_env::hwnd_edit_console, EM_SCROLLCARET, 0, 0) ;
	SendMessage(windows_env::hwnd_edit_console, EM_GETLINECOUNT, 0, 0); 
#else
	fputs(socket_buffer, stderr);
    fflush(stderr);
#endif
	if(handle_error)
        exit(0);
}
//***********************************************
namespace windows_env
{
#ifdef _WIN32
	extern HWND hwnd_edit_console = NULL;
	extern SOCKET main_socket = INVALID_SOCKET;
#endif
}
//***********************************************
bool isExecutable(const char* path)
{
#ifdef _WIN32
	return _access_s(path, 0x02) == 0;
#else
	return access(path, X_OK) == 0;
#endif
}
//***********************************************
size_t getFileSize(FILE* fp)
{
	long int oripos = ftell(fp);
	if(oripos == -1)
		error_handle("Fail to get the file offset");
	if(fseek(fp, 0, SEEK_END) != 0)
		error_handle("Fail to do seek on file");
	long int size = ftell(fp);
	if(fseek(fp, oripos, SEEK_SET) != 0)
		error_handle("Fail to do seek on file");
	if(size == -1)
		error_handle("Fail to get the file offset");
	return size;
}
//***********************************************
#ifdef _WIN32
AutoWinSocket::AutoWinSocket(): sock(INVALID_SOCKET)
{}
//***********************************************
AutoWinSocket::AutoWinSocket(SOCKET sock): sock(sock)
{}
//***********************************************
AutoWinSocket::AutoWinSocket(AutoWinSocket& other): sock(other.release())
{}
//***********************************************
AutoWinSocket::~AutoWinSocket()
{
	if(sock != INVALID_SOCKET)
		closesocket(sock);
}
//***********************************************
void AutoWinSocket::close()
{
	if(sock != INVALID_SOCKET)
	{
		if(closesocket(sock) != 0)
			error_handle("Fail to close the socket");
		sock = INVALID_SOCKET;
	}
}
//***********************************************
SOCKET AutoWinSocket::get() const
{
	return sock;
}
//***********************************************
AutoWinSocket& AutoWinSocket::operator=(AutoWinSocket& other)
{
	close();
	sock = other.release();
	return *this;
}
//***********************************************
SOCKET AutoWinSocket::release()
{
	const SOCKET old = sock;
	sock = INVALID_SOCKET;
	return old;
}
//***********************************************
AutoWinSocket::operator SOCKET() const
{
	return sock;
}
//***********************************************
#endif
