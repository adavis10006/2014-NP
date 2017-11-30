#include "client_list.h"
//***********************************************
int CLIENT_LIST::find_smallest()
{
	int client_id = 1;
	for(int i = 0; i < client_list.size(); i++)
		if(client_id == client_list[i]->client_id)
			client_id++;
		else
			return client_id;
//*---------------------------------------------*	
	return client_id;
}
//***********************************************
int CLIENT_LIST::find_index(int client_id)
{
	for(int i = 0; i < client_list.size(); i++)
		if(client_id == client_list[i]->client_id)
			return i;
//*---------------------------------------------*	
	return -1;
}
//***********************************************
int CLIENT_LIST::size()
{
	return client_list.size();
}
//***********************************************
void CLIENT_LIST::login(int client_id, CLIENT *client)
{
	client_list.insert(client_list.begin() + client_id - 1, client);
//*---------------------------------------------*	
	char send_buffer[BUFFER];
	int client_index = find_index(client_id);
//*---------------------------------------------*	
	for(int i = 0; i < client_list.size(); i++)
	{
		sprintf(send_buffer, "*** User '%s' entered from %s. ***\n", client_list[client_index]->name, client_list[client_index]->ip_port);
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
	}
}
//***********************************************
void CLIENT_LIST::logout(int client_id)
{
	char send_buffer[BUFFER];
	int client_index = find_index(client_id);
//*---------------------------------------------*	
	for(int i = 0; i < client_list.size(); i++)
	{
		sprintf(send_buffer, "*** User '%s' left. ***\n", client_list[client_index]->name);
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
	}
//*---------------------------------------------*		
	for(std::deque<MESSAGE*>::iterator it = message_list.begin(); it != message_list.end(); it++)
		if((*it)->outset_id == client_id || (*it)->destination_id == client_id)
		{	
			close((*it)->FIFO_read_fd);
			unlink((*it)->FIFO_name);
//*---------------------------------------------*		
			it = message_list.erase(it), it--;
		}
//*---------------------------------------------*		
#if defined(DEBUG_MESSAGE)
	printf("message: %zu\n", message_list.size());
	for(int i = 0; i < message_list.size(); i++)
		printf("%s\n", message_list[i]->FIFO_name);
#endif
	client_list.erase(client_list.begin() + client_index);
}
//***********************************************
void CLIENT_LIST::who(int socket_fd, int client_id)
{
	char send_buffer[BUFFER];
//*---------------------------------------------*		   
	sprintf(send_buffer, "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
	safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
	for(int i = 0; i < client_list.size(); i++, safety_write(socket_fd, "\n", strlen("\n")))
	{	
		sprintf(send_buffer, "%d\t%s\t%s\t", client_list[i]->client_id, client_list[i]->name, client_list[i]->ip_port);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		if(client_id == client_list[i]->client_id)
			safety_write(socket_fd, "<-me", strlen("<-me"));
	}	
}
//***********************************************
void CLIENT_LIST::tell(int socket_fd, int outset_id, int destination_id, const char *message)
{
	char send_buffer[BUFFER];
	int outset_index = find_index(outset_id), destination_index = find_index(destination_id);
//*---------------------------------------------*	
	int message_index;
	for(int i = 0, j = 0; i < strlen(message); i++)
	{	
		if(message[i] == ' ')
			j++;
		if(j == 2)
		{
			message_index = i + 1;
			break;
		}
	}
//*---------------------------------------------*
	if(destination_index == -1)
	{
		sprintf(send_buffer, "*** Error: user #%d does not exist yet. ***\n", destination_id);
		safety_write(client_list[outset_index]->socket_fd, send_buffer, strlen(send_buffer));
	}
	else
	{
		sprintf(send_buffer, "*** %s told you ***: %s\n", client_list[outset_index]->name, message + message_index);
		safety_write(client_list[destination_index]->socket_fd, send_buffer, strlen(send_buffer));
	}
}
//***********************************************
void CLIENT_LIST::yell(int socket_fd, int client_id, const char *message)
{
	char send_buffer[BUFFER];
	int client_index = find_index(client_id);
//*---------------------------------------------*	
	int message_index;
	for(int i = 0, j = 0; i < strlen(message); i++)
	{	
		if(message[i] == ' ')
			j++;
		if(j == 1)
		{
			message_index = i + 1;
			break;
		}
	}
//*---------------------------------------------*	
	sprintf(send_buffer, "*** %s yelled ***: %s\n", client_list[client_index]->name, message + message_index);
//*---------------------------------------------*	
	for(int i = 0; i < client_list.size(); i++)
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
}
//***********************************************
void CLIENT_LIST::name(int socket_fd, int client_id, char *name)
{
	char send_buffer[BUFFER];
	int client_index = find_index(client_id);
//*---------------------------------------------*		   
	for(int i = 0; i < client_list.size(); i++)
		if(strcmp(client_list[i]->name, name) == 0)
		{
			sprintf(send_buffer, "*** User '%s' already exists. ***\n", name);
			safety_write(socket_fd, send_buffer, strlen(send_buffer));
			return;
		}
//*---------------------------------------------*		   
	strcpy(client_list[client_index]->name, name);
//*---------------------------------------------*		   
	sprintf(send_buffer, "*** User from %s is named '%s'. ***\n", client_list[client_index]->ip_port, client_list[client_index]->name);
//*---------------------------------------------*		   
	for(int i = 0; i < client_list.size(); i++)
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
}
//***********************************************
bool CLIENT_LIST::check_client_to_stdin(int socket_fd, int outset_id, int destination_id, int& outset_fd)
{
	debug_check_client_to_stdin(socket_fd, outset_id, destination_id);
//*---------------------------------------------*
	int message_index = -1;
	for(int i = 0; i < message_list.size(); i++)
		if(message_list[i]->outset_id == outset_id && message_list[i]->destination_id == destination_id)
		{
			message_index = i;
			break;
		}
//*---------------------------------------------*	
	if(message_index == -1)
	{
		char send_buffer[BUFFER];
//*---------------------------------------------*		   
		sprintf(send_buffer, "*** Error: the pipe #%d->#%d does not exist yet. ***\n", outset_id, destination_id);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		return false;
	}
//*---------------------------------------------*   
	outset_fd = message_list[message_index]->FIFO_read_fd;
//*---------------------------------------------*   
	return true;
}
//***********************************************
bool CLIENT_LIST::check_stdout_to_client(int socket_fd, int outset_id, int destination_id, int& destination_fd)
{
	debug_check_stdout_to_client(socket_fd, outset_id, destination_id);
//*---------------------------------------------*		   
	int destination_index = find_index(destination_id);
//*---------------------------------------------*		   
	if(destination_index == -1)
	{
		char send_buffer[BUFFER];
//*---------------------------------------------*		   
		sprintf(send_buffer, "*** Error: user #%d does not exist yet. ***\n", destination_id);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		return false;
	}	
//*---------------------------------------------*		   
	for(int i = 0; i < message_list.size(); i++)
		if(message_list[i]->outset_id == outset_id && message_list[i]->destination_id == destination_id)
		{
			char send_buffer[BUFFER];
//*---------------------------------------------*		   
			sprintf(send_buffer, "*** Error: the pipe #%d->#%d already exists. ***\n", outset_id, destination_id);
			safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
			return false;
		}
//*---------------------------------------------*
	char FIFO_name[BUFFER];
	sprintf(FIFO_name, "/tmp/%d_to_%d_fifo", outset_id, destination_id);
//*---------------------------------------------*	
	if(mkfifo(FIFO_name, O_CREAT | FILE_MODE) < 0)
	{	
		error("FIFO", false);
		unlink(FIFO_name);
#if defined(DEBUG_FIFO)
		printf("[SERV]|FIFO| Request fail so unlink FIFO\n");
#endif
		if(mkfifo(FIFO_name, O_CREAT | FILE_MODE) < 0)
			return false;
#if defined(DEBUG_FIFO)
		printf("[SERV]|FIFO| Re-request FIFO\n");
#endif
	}
//*---------------------------------------------*   
	int FIFO_read_fd;
//*---------------------------------------------*
	if((FIFO_read_fd = open(FIFO_name, O_RDONLY | O_NONBLOCK)) < 0)
	{	
		error("read FIFO", false);
		return false;
	}
	if((destination_fd = open(FIFO_name, O_WRONLY | O_NONBLOCK)) < 0)
	{	
		error("write FIFO", false);
		return false;
	}
//*---------------------------------------------*
	message_list.push_back(new MESSAGE(outset_id, destination_id, FIFO_name, FIFO_read_fd));
	return true;
}
//***********************************************
void CLIENT_LIST::redirect_client_to_stdin(int FIFO_read_fd)
{
	debug_redirect_client_to_stdin(FIFO_read_fd);
//*---------------------------------------------*  
	dup2(FIFO_read_fd, STDIN_FILENO);
	close(FIFO_read_fd);
}
//***********************************************
void CLIENT_LIST::redirect_stdout_to_client(int FIFO_write_fd)
{
	debug_redirect_stdout_to_client(FIFO_write_fd);
//*---------------------------------------------*		   
	dup2(FIFO_write_fd, STDOUT_FILENO);
	dup2(FIFO_write_fd, STDERR_FILENO);
	close(FIFO_write_fd);
}
//***********************************************
void CLIENT_LIST::close_client_to_stdin(int FIFO_read_fd)
{
	debug_close_client_to_stdin(FIFO_read_fd);
//*---------------------------------------------*   
	close(FIFO_read_fd);
}
//***********************************************
void CLIENT_LIST::close_stdout_to_client(int FIFO_write_fd)
{
	debug_close_stdout_to_client(FIFO_write_fd);
//*---------------------------------------------*   
	close(FIFO_write_fd);
}
//***********************************************
void CLIENT_LIST::unlink_client_to_stdin(int FIFO_read_fd, int outset_id, int destination_id, char *command_string)
{
	debug_unlink_client_to_stdin(outset_id, destination_id, command_string);
//*---------------------------------------------*
	close(FIFO_read_fd);
//*---------------------------------------------*
	int message_index = -1;
	for(int i = 0; i < message_list.size(); i++)
		if(message_list[i]->outset_id == outset_id && message_list[i]->destination_id == destination_id)
		{
			message_index = i;
			break;
		}
//*---------------------------------------------*
	unlink(message_list[message_index]->FIFO_name);
//*---------------------------------------------*	
	char send_buffer[BUFFER];
	int outset_index = find_index(outset_id), destination_index = find_index(destination_id);
//*---------------------------------------------*		
	sprintf(send_buffer, "*** %s (#%d) just received from %s (#%d) by '%s' ***\n", client_list[destination_index]->name, destination_id, client_list[outset_index]->name, outset_id, command_string);
//*---------------------------------------------*		
	for(int i = 0; i < client_list.size(); i++)
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		
	message_list.erase(message_list.begin() + message_index);
}
//***********************************************
void CLIENT_LIST::broacast_stdout_to_client(int FIFO_write_fd, int outset_id, int destination_id, char *command_string)
{
	debug_broadcast_stdout_to_client(outset_id, destination_id, command_string);
//*---------------------------------------------*
	close(FIFO_write_fd);
//*---------------------------------------------*
	char send_buffer[BUFFER];
	int outset_index = find_index(outset_id), destination_index = find_index(destination_id);
//*---------------------------------------------*		
	sprintf(send_buffer, "*** %s (#%d) just piped '%s' to %s (#%d) ***\n", client_list[outset_index]->name, outset_id, command_string, client_list[destination_index]->name, destination_id);
//*---------------------------------------------*		
	for(int i = 0; i < client_list.size(); i++)
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
}
//***********************************************
CLIENT* CLIENT_LIST::operator[](int index)
{
	return client_list[index];
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