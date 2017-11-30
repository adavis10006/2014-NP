#include "server.h"
//***********************************************
char command_prompt[] = "% ";
//***********************************************
int main(int argc, char **argv)
{
	socklen_t client_len;
    int server_port;
	struct sockaddr_in server_addr, client_addr;
    int socket_fd, connect_fd;
	pid_t child_pid;
//*---------------------------------------------*	
	if(argc == 1)
		server_port = SERVER_PORT;
	else if(argc == 2)
        server_port = safety_port(atoi(argv[1]));
    else
        error("argc");
//*---------------------------------------------*	
	printf("[SERV] server version %s starting\n", SERVER_VERSION);
//*---------------------------------------------*		
	chdir(RAS_FILE);
//*---------------------------------------------*	
    socket_fd = stafety_socket(AF_INET, SOCK_STREAM, 0);
//*---------------------------------------------*	
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);
//*---------------------------------------------*	
    safety_bind(socket_fd, (SA*) &server_addr, sizeof(server_addr));
//*---------------------------------------------*	
    safety_listen(socket_fd, MAX_CLIENTS);
//*---------------------------------------------*	
	CLIENT_LIST client_list;
	std::deque<COMMAND_LIST*> command_list_set;
//*---------------------------------------------*	
	int nready;
	int maxfd = socket_fd;
	int maxi = -1;
	fd_set rset, allset;
//*---------------------------------------------*	
	FD_ZERO(&allset);
    FD_SET(socket_fd, &allset);
//*---------------------------------------------*		
    while(true)
    {
		rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
//*---------------------------------------------*	        
		if(FD_ISSET(socket_fd, &rset))
		{
			client_len = sizeof(client_addr);
			connect_fd = accept(socket_fd, (SA*) &client_addr, &client_len);
//*---------------------------------------------*				
			if(connect_fd == -1)
				error("accept", false);
//*---------------------------------------------*	
			int client_id = client_list.find_smallest();
//*---------------------------------------------*	
			debug_client_connect(client_id);
//*---------------------------------------------*	
			char welcome_string[] = "****************************************\n** Welcome to the information server. **\n****************************************\n";
//*---------------------------------------------*
			debug_welcome();
//*---------------------------------------------*
			safety_write(connect_fd, welcome_string, strlen(welcome_string));
//*---------------------------------------------*		
			client_list.login(client_id, new CLIENT(client_id, connect_fd, client_addr));
			command_list_set.insert(command_list_set.begin() + client_id - 1, new COMMAND_LIST(client_id, connect_fd, &client_list));
//*---------------------------------------------*		
			safety_write(connect_fd, command_prompt, strlen(command_prompt));
//*---------------------------------------------*		
            FD_SET(connect_fd, &allset);
//*---------------------------------------------*		
			if(connect_fd > maxfd)
                maxfd = connect_fd;
//*---------------------------------------------*		
            if(--nready <= 0)
                continue;		
		}
//*---------------------------------------------*		
		for(int i = 0; i < client_list.size(); i++)
        {
			if((connect_fd = client_list[i]->socket_fd) < 0)
                continue;
//*---------------------------------------------*		
			if(FD_ISSET(connect_fd, &rset))
            {
				debug_select(client_list[i]);
//*---------------------------------------------*	
				READLINE_RETURN readline_return = READLINE_UNDONE;
				char *receive = new char[READ_BUFFER];
				std::string command("");
//*---------------------------------------------*
				while(readline_return == READLINE_UNDONE)
					if((readline_return = safety_readline(connect_fd, receive, READ_BUFFER)) == READLINE_UNDONE)
						command.append(receive);
					else if(readline_return == READLINE_FINISH)
						command.append(receive);
					else if(readline_return == READLINE_EOF)
					{
						close(connect_fd);
						FD_CLR(connect_fd, &allset);
//*---------------------------------------------*		
						debug_client_terminate(client_list[i]->client_id);
//*---------------------------------------------*		
						client_list.logout(client_list[i]->client_id);
						command_list_set.erase(command_list_set.begin() + i);
					}
					else
						error("readline", false);
//*---------------------------------------------*	
				delete(receive);
//*---------------------------------------------*	
				debug_readline(command.c_str());
//*---------------------------------------------*
				if(readline_return != READLINE_EOF)
				{
					command_list_set[i]->new_command(command.c_str());
					COMMAND_RETURN command_return = command_list_set[i]->exec();
					command_list_set[i]->clear_command_list();
//*---------------------------------------------*						
					if(command_return == COMMAND_EXIT)
					{
						close(connect_fd);
						FD_CLR(connect_fd, &allset);
//*---------------------------------------------*		
						debug_client_terminate(client_list[i]->client_id);
//*---------------------------------------------*		
						client_list.logout(client_list[i]->client_id);
						command_list_set.erase(command_list_set.begin() + i);
			
					}
					safety_write(connect_fd, command_prompt, strlen(command_prompt));
				}
				if(--nready <= 0)
                    break;
			}
		}
	}
    return 0;
}
//***********************************************
void debug_select(CLIENT *client)
{
#if defined(DEBUG_SELECT)
	printf("[SERV]|SELE| Detect input from client: %d socket: %d ip: %s name %s\n", client->client_id, client->socket_fd, client->ip_port, client->name);
#endif
}
//***********************************************
