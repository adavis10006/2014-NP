#ifndef __CLIENT_LIST__
#define __CLIENT_LIST__
//************************************************
#include "socket_header.h"
#include "safety_function.h"
#include "environment_list.h"
#include "semaphore.h"
//***********************************************
struct MESSAGE
{
	int outset_id, destination_id;
	MESSAGE_STATE message_state;
	int FIFO_read_fd;
	char FIFO_name[BUFFER];
};
//***********************************************
struct CLIENT
{
	int client_id;
	int socket_fd;
	bool in_used;
	char name[25];
	char ip_port[30];
	MESSAGE message[30];
	int chat_index;
	char chat_buffer[MAX_CHAT][1024];
};
//***********************************************
void client_list_clear();
//***********************************************
int client_list_find_smallest();
//***********************************************
bool client_list_check_logout(int client_id);
//***********************************************
void client_list_send_message(int client_id);
//***********************************************
void client_list_login(int client_id, int socket_fd, struct sockaddr_in client_addr);
//***********************************************
void client_list_logout(int client_id);
//***********************************************
void client_list_who(int socket_fd, int client_id);
//***********************************************
void client_list_tell(int socket_fd, int outset_id, int destination_id, const char *message);
//***********************************************
void client_list_yell(int socket_fd, int client_id, const char *message);
//***********************************************
void client_list_name(int socket_fd, int client_id, char *name);
//***********************************************
bool client_list_check_client_to_stdin(int socket_fd, int outset_id, int destination_id, int& outset_fd);
//***********************************************
bool client_list_check_stdout_to_client(int socket_fd, int outset_id, int destination_id, int& destination_fd);
bool client_list_check_stdout_to_client_fd(int socket_fd, int outset_id, int destination_id, int& destination_fd);
//***********************************************
void client_list_redirect_client_to_stdin(int FIFO_read_fd);
//***********************************************
void client_list_redirect_stdout_to_client(int FIFO_write_fd);
//***********************************************
void client_list_close_client_to_stdin(int FIFO_read_fd);
//***********************************************
void client_list_close_stdout_to_client(int FIFO_write_fd);
//***********************************************
void client_list_unlink_client_to_stdin(int FIFO_read_fd, int outset_id, int destination_id, char *command_string);
//***********************************************
void client_list_broacast_stdout_to_client(int FIFO_write_fd, int outset_id, int destination_id, char *command_string);
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
