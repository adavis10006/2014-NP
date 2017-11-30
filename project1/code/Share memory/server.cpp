#include "server.h"
//***********************************************
void RAS(int socket_fd)
{
	char welcome_string[] = "****************************************\n** Welcome to the information server. **\n****************************************\n";
	char command_prompt[] = "% ";
	COMMAND_LIST command_list(socket_fd);
//-----------------------------------------------
	chdir("/home/davis/RAS");
	setenv("PATH", "bin:.", 1);
	setenv("PWD", "/home/davis/RAS", 1);
//-----------------------------------------------
	debug_welcome();
//-----------------------------------------------
	safety_write(socket_fd, welcome_string, strlen( welcome_string));
//-----------------------------------------------		
	while(true)
	{
		safety_write(socket_fd, command_prompt, strlen( command_prompt));
//-----------------------------------------------
		READLINE_RETURN readline_return = READLINE_UNDONE;
		char *receive = new char[READ_BUFFER];
		std::string command("");
//-----------------------------------------------
		while(readline_return == READLINE_UNDONE)
			if((readline_return = safety_readline(socket_fd, receive, READ_BUFFER)) == READLINE_UNDONE)
				command.append(receive);
			else if(readline_return == READLINE_FINISH)
				command.append(receive);
			else
				error("readline", false);
//-----------------------------------------------	
		delete(receive);
//-----------------------------------------------	
		debug_readline(command.c_str());
//-----------------------------------------------			
		command_list.new_command(command.c_str());
		COMMAND_RETURN command_return = command_list.exec();
		command_list.clear_command_list();
//-----------------------------------------------						
		if(command_return == COMMAND_EXIT)
			return;
	}
}
//***********************************************
int main(int argc, char **argv)
{
	socklen_t client_len;
    int server_port;
	struct sockaddr_in server_addr, client_addr;
    int socket_fd, connect_fd;
	pid_t child_pid;
//-----------------------------------------------	
	if(argc == 1)
		server_port = SERVER_PORT;
	else if(argc == 2)
        server_port = safety_port(atoi(argv[1]));
    else
        error("argc");
//-----------------------------------------------	
	printf("[SERV] server version %s starting\n", SERVER_VERSION);
//-----------------------------------------------	
    socket_fd = stafety_socket(AF_INET, SOCK_STREAM, 0);
//-----------------------------------------------	
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);
//-----------------------------------------------	
    safety_bind(socket_fd, (SA*) &server_addr, sizeof(server_addr));
//-----------------------------------------------	
    safety_listen(socket_fd, MAX_CLIENTS);
//-----------------------------------------------	
    safety_signal(SIGCHLD, sig_chld_server);
//-----------------------------------------------		
	int client_amount = 0;
//-----------------------------------------------		
    while(true)
    {
        client_len = sizeof(client_addr);
        connect_fd = accept(socket_fd, (SA*) &client_addr, &client_len);
//-----------------------------------------------				
		if(connect_fd == -1)
			error("accept", false);
//-----------------------------------------------		
		int client_amount = 0;
//-----------------------------------------------		
		debug_client_connect(++client_amount);
//-----------------------------------------------		
		if((child_pid = fork()) == 0)
        {
            close(socket_fd);
            RAS(connect_fd);
//-----------------------------------------------		
			debug_client_terminate(client_amount);
//-----------------------------------------------		
			close(connect_fd);
//-----------------------------------------------		
            return 0;
        }
		else if(child_pid > 0)
		{
			debug_fork_information(getpid(), child_pid);
//-----------------------------------------------		
			close(connect_fd);
		}
		else
			error("fork", false);
	}
//-----------------------------------------------		   
    return 0;
}
//***********************************************
