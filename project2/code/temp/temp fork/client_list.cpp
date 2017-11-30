#include "client_list.h"
//***********************************************
extern CLIENT *client_list;
extern int fifo_id;
//***********************************************
void client_list_clear()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		client_list[i].client_id = i + 1;
		client_list[i].socket_fd = -1;
		client_list[i].in_used = false;
		client_list[i].chat_index = 0;
//*---------------------------------------------*	
		for(int j = 0; j < MAX_CLIENTS; j++)
		{	
			client_list[i].message[j].outset_id = i;	
			client_list[i].message[j].destination_id = j;	
			client_list[i].message[j].message_state = MESSAGE_UNUSED;	
			client_list[i].message[j].FIFO_read_fd = -1;	
//*---------------------------------------------*	
			char name_buffer[BUFFER];
			sprintf(name_buffer, "/tmp/%d_to_%d_fifo", i, j);
			strcpy(client_list[i].message[j].FIFO_name, name_buffer);
		}
	}
}
//***********************************************
int client_list_find_smallest()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[i].socket_fd == -1)
			return i + 1;
}
//***********************************************
bool client_list_check_logout(int client_id)
{
	return client_list[client_id - 1].in_used;
}
//***********************************************
void client_list_send_message(int client_id)
{
	int client_index = client_id - 1;
//*---------------------------------------------*	
	if(client_list[client_index].socket_fd != -1)
		for(int i = 0; i < client_list[client_index].chat_index; i++)
			safety_write(client_list[client_index].socket_fd, client_list[client_index].chat_buffer[i], strlen(client_list[client_index].chat_buffer[i]));
//*---------------------------------------------*	
	if(client_list[client_index].chat_index > 0)
		printf("client %d chat %d socket %d\n", client_id, client_list[client_index].chat_index, client_list[client_index].socket_fd);

	client_list[client_index].chat_index = 0;	
}
//***********************************************
void client_list_check_fifo(int client_id)
{
	int client_index = client_id - 1;
//*---------------------------------------------*	
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[client_index].message[i].message_state == MESSAGE_INIT)
		{	
			int FIFO_read_fd;
//*---------------------------------------------*
			if((FIFO_read_fd = open(client_list[client_index].message[i].FIFO_name, O_RDONLY | O_NONBLOCK)) < 0)
			{	
				error("read FIFO", false);
				return;
			}
			client_list[client_index].message[i].FIFO_read_fd = FIFO_read_fd;
			client_list[client_index].message[i].message_state = MESSAGE_USED;
			printf("open fd %d from %d to %d\n", FIFO_read_fd, client_id, i + 1);
		}
}
//***********************************************
void client_list_login(int client_id, int socket_fd, struct sockaddr_in client_addr)
{
	printf("login socket_fd %d\n", socket_fd);
	int client_index = client_id - 1;
//*---------------------------------------------*	
	char servip[BUFFER];
	socklen_t len = sizeof(client_addr);
//*---------------------------------------------*
	client_list[client_index].socket_fd = socket_fd;
	client_list[client_index].in_used = true;
	client_list[client_index].chat_index = 0;
	strcpy(client_list[client_index].name, "(no name)");
	sprintf(client_list[client_index].ip_port, "%s/%d", inet_ntop(AF_INET, &client_addr.sin_addr, servip, sizeof(servip)), ntohs(client_addr.sin_port));
//*---------------------------------------------*	
	char send_buffer[BUFFER];	
//*---------------------------------------------*	
	sprintf(send_buffer, "*** User '%s' entered from %s. ***\n", client_list[client_index].name, client_list[client_index].ip_port);			
//*---------------------------------------------*	
	safety_write(socket_fd, send_buffer, strlen(send_buffer));
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[i].socket_fd != -1 && i != client_index)
			strcpy(client_list[i].chat_buffer[client_list[i].chat_index++], send_buffer);
}
//***********************************************
void client_list_logout(int client_id)
{
	printf("logout id %d\n", client_id);
	char send_buffer[BUFFER];
	int client_index = client_id - 1;
//*---------------------------------------------*		
	client_list[client_index].socket_fd = -1;
	client_list[client_index].in_used = false;
	client_list[client_index].chat_index = 0;
//*---------------------------------------------*	
	sprintf(send_buffer, "*** User '%s' left. ***\n", client_list[client_index].name);
//*---------------------------------------------*	
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[i].socket_fd != -1 && i != client_index)
		{
			printf("send id %d socket_fd %d\n", i + 1, client_list[i].socket_fd);
			strcpy(client_list[i].chat_buffer[client_list[i].chat_index++], send_buffer);
		}
//*---------------------------------------------*		
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(client_list[i].message[client_index].FIFO_read_fd != -1)
		{
			close(client_list[i].message[client_index].FIFO_read_fd);
			unlink(client_list[i].message[client_index].FIFO_name);
			client_list[i].message[client_index].FIFO_read_fd = -1;
			client_list[i].message[client_index].message_state = MESSAGE_UNUSED;	
		}
//*---------------------------------------------*		
		if(client_list[client_index].message[i].FIFO_read_fd != -1)
		{
			close(client_list[client_index].message[i].FIFO_read_fd);
			unlink(client_list[client_index].message[i].FIFO_name);
			client_list[client_index].message[i].FIFO_read_fd = -1;
			client_list[client_index].message[i].message_state = MESSAGE_UNUSED;	
		}
	}
//*---------------------------------------------*		
#if defined(DEBUG_MESSAGE)
	for(int i = 0; i < MAX_CLIENTS; i++)
		for(int j = 0; j < MAX_CLIENTS; j++)
			if(client_list[i].message[j].FIFO_read_fd != -1)
				printf("%s\n", client_list[i].message[j].FIFO_name);
#endif
}
//***********************************************
void client_list_who(int socket_fd, int client_id)
{
	char send_buffer[BUFFER];
//*---------------------------------------------*		   
	sprintf(send_buffer, "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
	safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
	for(int i = 0; i < MAX_CLIENTS; i++)
	{	
		if(client_list[i].socket_fd == -1)
			continue;
		sprintf(send_buffer, "%d\t%s\t%s\t", client_list[i].client_id, client_list[i].name, client_list[i].ip_port);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		if(client_id == client_list[i].client_id)
			safety_write(socket_fd, "<-me", strlen("<-me"));
//*---------------------------------------------*		   
		safety_write(socket_fd, "\n", strlen("\n"));
	}	
}
//***********************************************
void client_list_tell(int socket_fd, int outset_id, int destination_id, const char *message)
{
	char send_buffer[BUFFER];
	int outset_index = outset_id - 1, destination_index = destination_id - 1;
//*---------------------------------------------*
	if(client_list[destination_index].socket_fd == -1)
	{
		sprintf(send_buffer, "*** Error: user #%d does not exist yet. ***\n", destination_id);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
	}
	else
	{
		sprintf(send_buffer, "*** %s told you ***: %s\n", client_list[outset_index].name, message);
		strcpy(client_list[destination_index].chat_buffer[client_list[destination_index].chat_index++], send_buffer);
	}
}
//***********************************************
void client_list_yell(int socket_fd, int client_id, const char *message)
{
	char send_buffer[BUFFER];
	int client_index = client_id - 1;
//*---------------------------------------------*	
	sprintf(send_buffer, "*** %s yelled ***: %s\n", client_list[client_index].name, message);
//*---------------------------------------------*	
	safety_write(socket_fd, send_buffer, strlen(send_buffer));
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[i].socket_fd != -1 && i != client_index)
			strcpy(client_list[i].chat_buffer[client_list[i].chat_index++], send_buffer);
}
//***********************************************
void client_list_name(int socket_fd, int client_id, char *name)
{
	char send_buffer[BUFFER];
	int client_index = client_id - 1;
//*---------------------------------------------*		   
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[i].socket_fd != -1 && strcmp(client_list[i].name, name) == 0)
		{
			sprintf(send_buffer, "*** User '%s' already exists. ***\n", name);
			safety_write(socket_fd, send_buffer, strlen(send_buffer));
			return;
		}
//*---------------------------------------------*		   
	strcpy(client_list[client_index].name, name);
//*---------------------------------------------*		   
	sprintf(send_buffer, "*** User from %s is named '%s'. ***\n", client_list[client_index].ip_port, client_list[client_index].name);
//*---------------------------------------------*		   
	safety_write(socket_fd, send_buffer, strlen(send_buffer));
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[i].socket_fd != -1 && i != client_index)
			strcpy(client_list[i].chat_buffer[client_list[i].chat_index++], send_buffer);
}
//***********************************************
bool client_list_check_client_to_stdin(int socket_fd, int outset_id, int destination_id, int& outset_fd)
{
	debug_check_client_to_stdin(socket_fd, outset_id, destination_id);
//*---------------------------------------------*
	if(client_list[outset_id - 1].message[destination_id - 1].FIFO_read_fd == -1)
	{
		char send_buffer[BUFFER];
//*---------------------------------------------*		   
		sprintf(send_buffer, "*** Error: the pipe #%d->#%d does not exist yet. ***\n", outset_id, destination_id);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		return false;
	}
//*---------------------------------------------*   
	close(client_list[outset_id - 1].message[destination_id - 1].FIFO_read_fd);
//*---------------------------------------------*   
	int FIFO_read_fd;
//*---------------------------------------------*   
	if((FIFO_read_fd = open(client_list[outset_id - 1].message[destination_id - 1].FIFO_name, O_RDONLY)) < 0)
		error("read FIFO", false);
//*---------------------------------------------*   
	outset_fd = FIFO_read_fd;
//*---------------------------------------------*   
	return true;
}
//***********************************************
bool client_list_check_stdout_to_client(int socket_fd, int outset_id, int destination_id, int& destination_fd)
{
	debug_check_stdout_to_client(socket_fd, outset_id, destination_id);
//*---------------------------------------------*		   
	int destination_index = destination_id - 1;
//*---------------------------------------------*		   
	if(client_list[destination_index].socket_fd == -1)
	{
		char send_buffer[BUFFER];
//*---------------------------------------------*		   
		sprintf(send_buffer, "*** Error: user #%d does not exist yet. ***\n", destination_id);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		return false;
	}	
//*---------------------------------------------*		   
	if(client_list[outset_id - 1].message[destination_id - 1].FIFO_read_fd != -1)
	{
		char send_buffer[BUFFER];
//*---------------------------------------------*		   
		sprintf(send_buffer, "*** Error: the pipe #%d->#%d already exists. ***\n", outset_id, destination_id);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		return false;
	}
//*---------------------------------------------*
	if(mkfifo(client_list[outset_id - 1].message[destination_id - 1].FIFO_name, O_CREAT | FILE_MODE) < 0)
	{	
		error("FIFO", false);
		unlink(client_list[outset_id - 1].message[destination_id - 1].FIFO_name);
#if defined(DEBUG_FIFO)
		printf("[SERV]|FIFO| Request fail so unlink FIFO\n");
#endif
		if(mkfifo(client_list[outset_id - 1].message[destination_id - 1].FIFO_name, O_CREAT | FILE_MODE) < 0)
			return false;
#if defined(DEBUG_FIFO)
		printf("[SERV]|FIFO| Re-request FIFO\n");
#endif
	}
//*---------------------------------------------*  
	client_list[outset_id - 1].message[destination_id - 1].message_state = MESSAGE_INIT;	
//*---------------------------------------------*
	return true;
}
//***********************************************
bool client_list_check_stdout_to_client_fd(int socket_fd, int outset_id, int destination_id, int& destination_fd)
{
	if(client_list[outset_id - 1].message[destination_id - 1].message_state != MESSAGE_USED)
		return false;
	else if((destination_fd = open(client_list[outset_id - 1].message[destination_id - 1].FIFO_name, O_WRONLY | O_NONBLOCK)) < 0)
	{	
		error("write FIFO", false);
		return false;
	}
	return true;
}
//***********************************************
void client_list_redirect_client_to_stdin(int FIFO_read_fd)
{
	debug_redirect_client_to_stdin(FIFO_read_fd);
//*---------------------------------------------*  
	dup2(FIFO_read_fd, STDIN_FILENO);
	close(FIFO_read_fd);
}
//***********************************************
void client_list_redirect_stdout_to_client(int FIFO_write_fd)
{
	debug_redirect_stdout_to_client(FIFO_write_fd);
//*---------------------------------------------*		   
	dup2(FIFO_write_fd, STDOUT_FILENO);
	dup2(FIFO_write_fd, STDERR_FILENO);
	close(FIFO_write_fd);
}
//***********************************************
void client_list_close_client_to_stdin(int FIFO_read_fd)
{
	debug_close_client_to_stdin(FIFO_read_fd);
//*---------------------------------------------*   
	close(FIFO_read_fd);
}
//***********************************************
void client_list_close_stdout_to_client(int FIFO_write_fd)
{
	debug_close_stdout_to_client(FIFO_write_fd);
//*---------------------------------------------*   
	close(FIFO_write_fd);
}
//***********************************************
void client_list_unlink_client_to_stdin(int FIFO_read_fd, int outset_id, int destination_id, char *command_string)
{
	debug_unlink_client_to_stdin(outset_id, destination_id, command_string);
//*---------------------------------------------*
	close(FIFO_read_fd);
//*---------------------------------------------*
	unlink(client_list[outset_id - 1].message[destination_id - 1].FIFO_name);
//*---------------------------------------------*	
	char send_buffer[BUFFER];
	int outset_index = outset_id - 1, destination_index = destination_id - 1;
//*---------------------------------------------*		
	sprintf(send_buffer, "*** %s (#%d) just received from %s (#%d) by '%s' ***\n", client_list[destination_index].name, destination_id, client_list[outset_index].name, outset_id, command_string);
//*---------------------------------------------*		
	safety_write(client_list[destination_index].socket_fd, send_buffer, strlen(send_buffer));
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[i].socket_fd != -1 && i != destination_index)
			strcpy(client_list[i].chat_buffer[client_list[i].chat_index++], send_buffer);
//*---------------------------------------------*		
	client_list[outset_id - 1].message[destination_id - 1].FIFO_read_fd = -1;
	client_list[outset_id - 1].message[destination_id - 1].message_state = MESSAGE_UNUSED;	
}
//***********************************************
void client_list_broacast_stdout_to_client(int FIFO_write_fd, int outset_id, int destination_id, char *command_string)
{
	debug_broadcast_stdout_to_client(outset_id, destination_id, command_string);
//*---------------------------------------------*
	close(FIFO_write_fd);
//*---------------------------------------------*
	char send_buffer[BUFFER];
	int outset_index = outset_id - 1, destination_index = destination_id - 1;	
//*---------------------------------------------*		
	sprintf(send_buffer, "*** %s (#%d) just piped '%s' to %s (#%d) ***\n", client_list[outset_index].name, outset_id, command_string, client_list[destination_index].name, destination_id);
//*---------------------------------------------*		
	safety_write(client_list[outset_index].socket_fd, send_buffer, strlen(send_buffer));
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(client_list[i].socket_fd != -1 && i != outset_index)
			strcpy(client_list[i].chat_buffer[client_list[i].chat_index++], send_buffer);
}
//***********************************************
void debug_check_client_to_stdin(int socket_fd, int outset_id, int destination_id)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Check client %d to stdin %d at socket %d\n", outset_id, destination_id, socket_fd);
#endif
}
//***********************************************
void debug_check_stdout_to_client(int socket_fd, int outset_id, int destination_id)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Check stdout %d to client %d at socket %d\n", outset_id, destination_id, socket_fd);
#endif
}
//***********************************************
void debug_redirect_client_to_stdin(int FIFO_read_fd)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Redirect client to stdin at openfd %d\n", FIFO_read_fd);
#endif
}
//***********************************************
void debug_redirect_stdout_to_client(int FIFO_write_fd)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Redirect stdout to client at openfd %d\n", FIFO_write_fd);
#endif
}
//***********************************************
void debug_close_client_to_stdin(int FIFO_read_fd)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Close client to stdin at openfd %d\n", FIFO_read_fd);
#endif
}
//***********************************************
void debug_close_stdout_to_client(int FIFO_write_fd)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Close stdout to client at openfd %d\n", FIFO_write_fd);
#endif
}
//***********************************************
void debug_unlink_client_to_stdin(int outset_id, int destination_id, char *command_string)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Unlink client %d to stdin %d command '%s'\n", outset_id, destination_id, command_string);
#endif
}
//***********************************************
void debug_broadcast_stdout_to_client(int outset_id, int destination_id, char *command_string)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Broadcast stdout %d to client %d command '%s'\n", outset_id, destination_id, command_string);
#endif
}
//***********************************************