#ifndef __CLIENT_LIST__
#define __CLIENT_LIST__
//************************************************
#include "socket_header.h"
#include "safety_function.h"
#include "environment_list.h"
//***********************************************
struct MESSAGE
{
	int outset_id, destination_id;
	int FIFO_read_fd;
	char FIFO_name[BUFFER];
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	MESSAGE(int outset_id, int destination_id, char *name, int FIFO_read_fd): outset_id(outset_id), destination_id(destination_id), FIFO_read_fd(FIFO_read_fd)
	{
		strcpy(FIFO_name, name);
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
		socklen_t len = sizeof(client_addr);
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
	int size();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void login(int client_id, CLIENT *client);
	void logout(int client_id);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void who(int socket_fd, int client_id);
	void tell(int socket_fd, int outset_id, int destination_id, const char *message);
	void yell(int socket_fd, int client_id, const char *message);
	void name(int socket_fd, int client_id, char *name);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	bool check_client_to_stdin(int socket_fd, int outset_id, int destination_id, int& outset_fd);
	bool check_stdout_to_client(int socket_fd, int outset_id, int destination_id, int& destination_fd);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void redirect_client_to_stdin(int FIFO_read_fd);
	void redirect_stdout_to_client(int FIFO_write_fd);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void close_client_to_stdin(int FIFO_read_fd);
	void close_stdout_to_client(int FIFO_write_fd);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void unlink_client_to_stdin(int FIFO_read_fd, int outset_id, int destination_id, char *command_string);
	void broacast_stdout_to_client(int FIFO_write_fd, int outset_id, int destination_id, char *command_string);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	CLIENT* operator[](int index);
private:
	std::deque<MESSAGE*> message_list;
	std::deque<CLIENT*> client_list;
};
//***********************************************
void debug_check_client_to_stdin(int socket_fd, int outset_id, int destination_id);
void debug_check_stdout_to_client(int socket_fd, int outset_id, int destination_id);
//***********************************************
void debug_redirect_client_to_stdin(int FIFO_read_fd);
void debug_redirect_stdout_to_client(int FIFO_write_fd);
//***********************************************
void debug_close_client_to_stdin(int FIFO_read_fd);
void debug_close_stdout_to_client(int FIFO_write_fd);
//***********************************************
void debug_unlink_client_to_stdin(int outset_id, int destination_id, char *command_string);
void debug_broadcast_stdout_to_client(int outset_id, int destination_id, char *command_string);
//***********************************************
#endif
