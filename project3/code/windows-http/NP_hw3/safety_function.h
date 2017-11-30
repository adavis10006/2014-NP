#ifndef __SAFETY_FUNCTION__
#define __SAFETY_FUNCTION__
//***********************************************
#include <stdarg.h>
#include "socket_header.h"
//***********************************************
size_t safety_readline(int fd, char *vptr, size_t max_len, bool handle_nonblocking = false);
size_t safety_write(int fd, char *vptr, size_t max_len);
//***********************************************
void web_output(int socket_fd, const char *output_string, ...);
void web_error(int socket_fd, const char *output_pointer, ...);
void web_script(int socket_fd, int client_id, const char *output_pointer, int output_len);
//***********************************************
void server_handle(int socket_fd, const char *debug_string, ...);
void debug_handle(const char *debug_string, ...);
void error_handle(const char *error_string, bool handle_exit = false, ...);
//***********************************************
namespace windows_env
{
#ifdef _WIN32
	extern HWND hwnd_edit_console;
	extern SOCKET main_socket;
#endif
}
//***********************************************
bool isExecutable(const char* path);
size_t getFileSize(FILE* fp);
//***********************************************
#ifdef _WIN32
class AutoWinSocket
{
private:
	SOCKET sock;
public:
	AutoWinSocket(AutoWinSocket& sock);
	AutoWinSocket(SOCKET sock);
	AutoWinSocket();
	~AutoWinSocket();
	SOCKET get() const;
	void close();
	AutoWinSocket& operator=(AutoWinSocket& newSock);
	SOCKET release();
	operator SOCKET() const;
};
//***********************************************
typedef SOCKET Socket;
typedef AutoWinSocket AutoSocket;
typedef int socklen_t;
//***********************************************
class SocketInfo
{
public:
	AutoSocket socket;
	struct sockaddr_storage sa;
	socklen_t saLen;
};
//***********************************************
#endif
//***********************************************
#endif