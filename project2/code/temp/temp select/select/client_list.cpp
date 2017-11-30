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
void CLIENT_LIST::insert(int index, CLIENT *client)
{
	client_list.insert(client_list.begin() + index, client);
}
//***********************************************
void CLIENT_LIST::erase(int index)
{
	client_list.erase(client_list.begin() + index);
}
//***********************************************
int CLIENT_LIST::size()
{
	return client_list.size();
}
//***********************************************
void CLIENT_LIST::login(int client_id)
{
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
	{
		if((*it)->from_client_id == client_id || (*it)->to_client_id == client_id)
			it = message_list.erase(it), it--;
	}
//*---------------------------------------------*		
	printf("message: %d\n", message_list.size());
	for(int i = 0; i < message_list.size(); i++)
		printf("%s\n", message_list[i]->name);
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
void CLIENT_LIST::tell(int socket_fd, int client_id, int transfer_id, const char *message)
{
	char send_buffer[BUFFER];
	int client_index = find_index(client_id), transfer_index = find_index(transfer_id);
//*---------------------------------------------*
	if(transfer_index == -1)
	{
		sprintf(send_buffer, "*** Error: user #%d does not exist yet. ***\n", transfer_id);
		safety_write(client_list[client_index]->socket_fd, send_buffer, strlen(send_buffer));
	}
	else
	{
		sprintf(send_buffer, "*** %s told you ***: %s\n", client_list[client_index]->name, message);
		safety_write(client_list[transfer_index]->socket_fd, send_buffer, strlen(send_buffer));
	}
}
//***********************************************
void CLIENT_LIST::yell(int socket_fd, int client_id, const char *message)
{
	char send_buffer[BUFFER];
	int client_index = find_index(client_id);
//*---------------------------------------------*		   
	for(int i = 0; i < client_list.size(); i++)
	{
		sprintf(send_buffer, "*** %s yelled ***: %s\n", client_list[client_index]->name, message);
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
	}
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
	for(int i = 0; i < client_list.size(); i++)
	{
		sprintf(send_buffer, "*** User from %s is named '%s'. ***\n", client_list[client_index]->ip_port, client_list[client_index]->name);
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
	}
}
//***********************************************
bool CLIENT_LIST::check_client_to_stdin(int socket_fd, int from_client_id, int client_id, int& from_client_fd)
{
	debug_check_client_to_stdin(socket_fd, from_client_id, client_id);
//*---------------------------------------------*
	int message_index = -1;
	for(int i = 0; i < message_list.size(); i++)
		if(message_list[i]->from_client_id == from_client_id && message_list[i]->to_client_id == client_id)
		{
			message_index = i;
			break;
		}
//*---------------------------------------------*	
	if(message_index == -1)
	{
		char send_buffer[BUFFER];
//*---------------------------------------------*		   
		sprintf(send_buffer, "*** Error: the pipe #%d->#%d does not exist yet. ***\n", from_client_id, client_id);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		return false;
	}
//*---------------------------------------------*   
	from_client_fd = message_list[message_index]->read_fd;
//*---------------------------------------------*   
	return true;
}
//***********************************************
bool CLIENT_LIST::check_stdout_to_client(int socket_fd, int client_id, int to_client_id, int& to_client_fd)
{
	debug_check_stdout_to_client(socket_fd, client_id, to_client_id);
//*---------------------------------------------*		   
	
	int to_client_index = find_index(to_client_id);
//*---------------------------------------------*		   
	if(to_client_index == -1)
	{
		char send_buffer[BUFFER];
//*---------------------------------------------*		   
		sprintf(send_buffer, "*** Error: user #%d does not exist yet. ***\n", to_client_id);
		safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
		return false;
	}	
//*---------------------------------------------*		   
	for(int i = 0; i < message_list.size(); i++)
		if(message_list[i]->from_client_id == client_id && message_list[i]->to_client_id == to_client_id)
		{
			char send_buffer[BUFFER];
//*---------------------------------------------*		   
			sprintf(send_buffer, "*** Error: the pipe #%d->#%d already exists. ***\n", client_id, to_client_id);
			safety_write(socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		   
			return false;
		}
//*---------------------------------------------*
	char name[BUFFER];
	sprintf(name, "/tmp/%d_to_%d_fifo", client_id, to_client_id);
//*---------------------------------------------*	
	if(mkfifo(name, O_CREAT | FILE_MODE) < 0)
	{	
		error("FIFO", false);
		unlink(name);
#if defined(DEBUG_FIFO)
		printf("[SERV]|FIFO| Request fail so unlink FIFO\n");
#endif
		if(mkfifo(name, O_CREAT | FILE_MODE) < 0)
			return false;
#if defined(DEBUG_FIFO)
		printf("[SERV]|FIFO| Re-request FIFO\n");
#endif
	}
//*---------------------------------------------*   
	int read_fd;
//*---------------------------------------------*
	if((read_fd = open(name, O_RDONLY | O_NONBLOCK)) < 0)
	{	
		error("read FIFO", false);
		return false;
	}
	if((to_client_fd = open(name, O_WRONLY | O_NONBLOCK)) < 0)
	{	
		error("write FIFO", false);
		return false;
	}
//*---------------------------------------------*
	message_list.push_back(new MESSAGE(client_id, to_client_id, name, read_fd));
	return true;
}
//***********************************************
void CLIENT_LIST::redirect_client_to_stdin(int from_client_fd)
{
	debug_redirect_client_to_stdin(from_client_fd);
//*---------------------------------------------*  
	dup2(from_client_fd, STDIN_FILENO);
	close(from_client_fd);
}
//***********************************************
void CLIENT_LIST::redirect_stdout_to_client(int to_client_fd)
{
	debug_redirect_stdout_to_client(to_client_fd);
//*---------------------------------------------*		   
	dup2(to_client_fd, STDOUT_FILENO);
	dup2(to_client_fd, STDERR_FILENO);
	close(to_client_fd);
}
//***********************************************
void CLIENT_LIST::close_client_to_stdin(int from_client_fd)
{
	debug_close_client_to_stdin(from_client_fd);
//*---------------------------------------------*   
	close(from_client_fd);
}
//***********************************************
void CLIENT_LIST::close_stdout_to_client(int to_client_fd)
{
	debug_close_stdout_to_client(to_client_fd);
//*---------------------------------------------*   
	close(to_client_fd);
}
//***********************************************
void CLIENT_LIST::unlink_client_to_stdin(int from_client_fd, int from_client_id, int client_id, char* command_string)
{
	debug_unlink_client_to_stdin(from_client_id, client_id, command_string);
//*---------------------------------------------*
	close(from_client_fd);
//*---------------------------------------------*
	int message_index = -1;
	for(int i = 0; i < message_list.size(); i++)
		if(message_list[i]->from_client_id == from_client_id && message_list[i]->to_client_id == client_id)
		{
			message_index = i;
			break;
		}
//*---------------------------------------------*
	unlink(message_list[message_index]->name);
//*---------------------------------------------*	
	char send_buffer[BUFFER];
	int client_index = find_index(client_id), from_client_index = find_index(from_client_id);
//*---------------------------------------------*		
	sprintf(send_buffer, "*** %s (#%d) just received from %s (#%d) by '%s' ***\n", client_list[client_index]->name, client_id, client_list[from_client_index]->name, from_client_id, command_string);
//*---------------------------------------------*		
	for(int i = 0; i < client_list.size(); i++)
		safety_write(client_list[i]->socket_fd, send_buffer, strlen(send_buffer));
//*---------------------------------------------*		
	message_list.erase(message_list.begin() + message_index);
}
//***********************************************
void CLIENT_LIST::broacast_stdout_to_client(int to_client_fd, int client_id, int to_client_id, char* command_string)
{
	debug_broadcast_stdout_to_client(client_id, to_client_id, command_string);
//*---------------------------------------------*
	close(to_client_fd);
//*---------------------------------------------*
	char send_buffer[BUFFER];
	int client_index = find_index(client_id), to_client_index = find_index(to_client_id);
//*---------------------------------------------*		
	sprintf(send_buffer, "*** %s (#%d) just piped '%s' to %s (#%d) ***\n", client_list[client_index]->name, client_id, command_string, client_list[to_client_index]->name, to_client_id);
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
void debug_check_client_to_stdin(int socket_fd, int from_client_id, int client_id)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Check client %d to stdin %d at socket %d\n", from_client_id, client_id, socket_fd);
#endif
}
//***********************************************
void debug_check_stdout_to_client(int socket_fd, int client_id, int to_client_id)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Check stdout %d to client %d at socket %d\n", client_id, to_client_id, socket_fd);
#endif
}
//***********************************************
void debug_redirect_client_to_stdin(int from_client_fd)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Redirect client to stdin at openfd %d\n", from_client_fd);
#endif
}
//***********************************************
void debug_redirect_stdout_to_client(int to_client_fd)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Redirect stdout to client at openfd %d\n", to_client_fd);
#endif
}
//***********************************************
void debug_close_client_to_stdin(int from_client_fd)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Close client to stdin at openfd %d\n", from_client_fd);
#endif
}
//***********************************************
void debug_close_stdout_to_client(int to_client_fd)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Close stdout to client at openfd %d\n", to_client_fd);
#endif
}
//***********************************************
void debug_unlink_client_to_stdin(int from_client_id, int client_id, char* command_string)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Unlink client %d to stdin %d command '%s'\n", from_client_id, client_id, command_string);
#endif
}
//***********************************************
void debug_broadcast_stdout_to_client(int client_id, int to_client_fd, char* command_string)
{
#if defined(DEBUG_FIFO) && defined(DEBUG_FIFO_REDIRECT)
	printf("[SERV]|FIFO| Broadcast stdout %d to client %d command '%s'\n", client_id, to_client_fd, command_string);
#endif
}