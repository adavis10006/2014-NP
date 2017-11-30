#ifndef __CLIENT_LIST__
#define __CLIENT_LIST__
//************************************************
#include "socket_header.h"
#include "safety_function.h"
#include "environment_list.h"
//***********************************************
struct MESSAGE
{
	int from_client_id, to_client_id;
	int read_fd;
	char name[BUFFER];
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	MESSAGE(int from_client_id, int to_client_id, char* FIFO_name, int read_fd): from_client_id(from_client_id), to_client_id(to_client_id), read_fd(read_fd)
	{
		strcpy(name, FIFO_name);
	}
};
//***********************************************
struct CLIENT
{
	int client_id;
	int socket_fd;
	char name[25];
	char ip_port[30];
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	CLIENT(int client_id, int socket_fd, struct sockaddr_in client_addr): client_id(client_id), socket_fd(socket_fd)
	{
		char servip[BUFFER];
		socklen_t len = sizeof( client_addr );
	//*---------------------------------------------*
		strcpy(name, "(no name)");
	//*---------------------------------------------*
		sprintf(ip_port, "%s/%d", inet_ntop(AF_INET, &client_addr.sin_addr, servip, sizeof(servip)), ntohs(client_addr.sin_port));
	}
};
//***********************************************
class CLIENT_LIST
{
public:
	int find_smallest();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	int find_index(int client_id);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	void insert(int index, CLIENT *client);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	void erase(int index);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	int size();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void login(int client_id);
	void logout(int client_id);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void who(int socket_fd, int client_id);
	void tell(int socket_fd, int client_id, int transfer_id, const char *message);
	void yell(int socket_fd, int client_id, const char *message);
	void name(int socket_fd, int client_id, char *name);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	bool check_client_to_stdin(int socket_fd, int from_client_id, int client_id, int& from_client_fd);
	bool check_stdout_to_client(int socket_fd, int client_id, int to_client_id, int& to_client_fd);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void redirect_client_to_stdin(int from_client_fd);
	void redirect_stdout_to_client(int to_client_fd);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void close_client_to_stdin(int from_client_fd);
	void close_stdout_to_client(int to_client_fd);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void unlink_client_to_stdin(int from_client_fd, int from_client_id, int client_id, char* command_string);
	void broacast_stdout_to_client(int to_client_fd, int client_id, int to_client_id, char* command_string);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	CLIENT* operator[](int index);
private:
	std::deque<MESSAGE*> message_list;
	std::deque<CLIENT*> client_list;
};
//***********************************************
void debug_check_client_to_stdin(int socket_fd, int from_client_id, int client_id);
void debug_check_stdout_to_client(int socket_fd, int client_id, int to_client_id);
//***********************************************
void debug_redirect_client_to_stdin(int from_client_fd);
void debug_redirect_stdout_to_client(int to_client_fd);
//***********************************************
void debug_close_client_to_stdin(int from_client_fd);
void debug_close_stdout_to_client(int to_client_fd);
//***********************************************
void debug_unlink_client_to_stdin(int from_client_id, int client_id, char* command_string);
void debug_broadcast_stdout_to_client(int client_id, int to_client_id, char* command_string);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*

#endif
