#include "server.h"
//***********************************************
CLIENT *client_list;
int shmid, sem_id, fifo_id;
//***********************************************
void RAS(int socket_fd, struct sockaddr_in client_addr)
{
	int write_pid;
	char welcome_string[] = "****************************************\n** Welcome to the information server. **\n****************************************\n";
	char command_prompt[] = "% ";
//*---------------------------------------------*
	sem_wait(sem_id);
	int client_id = client_list_find_smallest();
	sem_signal(sem_id);
//*---------------------------------------------*
	COMMAND_LIST command_list(client_id, socket_fd);
//*---------------------------------------------*
	chdir(RAS_FILE);
	clearenv();
	setenv("PATH", "bin:.", 1);
//*---------------------------------------------*
	debug_welcome();
//*---------------------------------------------*
	safety_write(socket_fd, welcome_string, strlen(welcome_string));
//*---------------------------------------------*		
	sem_wait(sem_id);
	client_list_login(client_id, socket_fd, client_addr);
	sem_signal(sem_id);
//*---------------------------------------------*	
	if((write_pid = fork()) == 0)
	{
		while(true)
		{
			safety_write(socket_fd, command_prompt, strlen(command_prompt));
	//*---------------------------------------------*
			READLINE_RETURN readline_return = READLINE_UNDONE;
			char *receive = new char[READ_BUFFER];
			std::string command("");
	//*---------------------------------------------*
			while(readline_return == READLINE_UNDONE)
				if((readline_return = safety_readline(socket_fd, receive, READ_BUFFER)) == READLINE_UNDONE)
					command.append(receive);
				else if(readline_return == READLINE_FINISH)
					command.append(receive);
				else if(readline_return == READLINE_EOF)
				{
					sem_wait(sem_id);
					client_list_logout(client_id);
					sem_signal(sem_id);
					return;
				}
				else
					error("readline", false);
	//*---------------------------------------------*	
			delete(receive);
	//*---------------------------------------------*	
			debug_readline(command.c_str());
	//*---------------------------------------------*			
			command_list.new_command(command.c_str());
			COMMAND_RETURN command_return = command_list.exec();
			command_list.clear_command_list();
	//*---------------------------------------------*						
			if(command_return == COMMAND_EXIT)
			{
				sem_wait(sem_id);
				client_list_logout(client_id);
				sem_signal(sem_id);
				return;
			}
		}
	}
	else if(write_pid > 0)
		while(true)
		{
			sem_wait(sem_id);
			bool handle_exit = client_list_check_logout(client_id);
			//sem_signal(sem_id);
			//sem_wait(sem_id);
			client_list_check_fifo(client_id);
			//sem_signal(sem_id);
			//sem_wait(sem_id);
			client_list_send_message(client_id);
			sem_signal(sem_id);
			if(!handle_exit)
				break;
		}
	else
		error("fork", false);
}
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
    safety_signal(SIGCHLD, sig_chld_server);
//*---------------------------------------------*	
	shmid = safety_shmget(IPC_PRIVATE, sizeof(CLIENT[MAX_CLIENTS]), IPC_CREAT | FILE_MODE);
	client_list = (CLIENT*) safety_shmat(shmid, NULL, 0);
//*---------------------------------------------*	
	sem_id = sem_create(SEM_KEY, 1);
	fifo_id = sem_create(FIFO_KEY, 0);
//*---------------------------------------------*		
	sem_wait(sem_id);
	client_list_clear();
	sem_signal(sem_id);
//*---------------------------------------------*	
	int client_amount = 0;
//*---------------------------------------------*	
    while(true)
    {
		client_len = sizeof(client_addr);
        connect_fd = accept(socket_fd, (SA*) &client_addr, &client_len);
//*---------------------------------------------*				
		if(connect_fd == -1)
			error("accept", false);
//*---------------------------------------------*		
			if((child_pid = fork()) == 0)
        {
			sem_wait(sem_id);
			int client_id = client_list_find_smallest();
			sem_signal(sem_id);
//*---------------------------------------------*		
			debug_client_connect(client_id);
//*---------------------------------------------*		
	        close(socket_fd);
		    RAS(connect_fd, client_addr);
//*---------------------------------------------*		
			debug_client_terminate(client_id);
//*---------------------------------------------*	
			close(connect_fd);
//*---------------------------------------------*		
            return 0;
        }
		else if(child_pid > 0)
		{
			debug_fork_information(getpid(), child_pid);
//*---------------------------------------------*		
			close(connect_fd);
		}
		else
			error("fork", false);
	}
//*---------------------------------------------*		  
	sem_close(sem_id);
	sem_close(fifo_id);
//*---------------------------------------------*		  
    stafety_shmctl(shmid,IPC_RMID, NULL);
	stafety_shmdt(client_list);  
//*---------------------------------------------*		
	return 0;
}
//***********************************************
