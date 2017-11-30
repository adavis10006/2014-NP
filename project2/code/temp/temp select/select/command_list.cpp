#include "command_list.h"
//***********************************************
struct COMMAND
{
	char *command;
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	COMMAND(char *buffer, int len)
	{
		command = new char[len + 1];
//*---------------------------------------------*    
		strncpy(command, buffer, len);
		command[len] = '\0';
//*---------------------------------------------*    
		debug_command_constructor(this, command);
	}
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	~COMMAND()
	{
		debug_command_destructor(this);
//*---------------------------------------------*    		
		delete(command);
	}
};
//***********************************************
COMMAND_LIST::COMMAND_LIST(int client_id, int socket_fd, CLIENT_LIST *client_list): client_id(client_id), socket_fd(socket_fd), command_string(NULL), environment_list(), client_list(client_list)
{
	debug_command_list_constructor(this);
}
//***********************************************
COMMAND_LIST::~COMMAND_LIST()
{
	debug_command_list_destructor(this);
//*---------------------------------------------*    		
	clear_pipe_vector();
	clear_command_list();
}
//***********************************************
void COMMAND_LIST::new_command(const char *command)
{
	debug_new_command();
//*---------------------------------------------*
	if(command_string != NULL)
		delete(command_string);
//*---------------------------------------------*
	command_string = new char[strlen(command) + 1];
	strcpy(command_string, command);
//*---------------------------------------------*
	parse_command(command);
	check_pipe_vector(0);
}
//***********************************************
void COMMAND_LIST::parse_command(const char* command)
{
	char temp[SINGLE_COMMAND_MAXBUFFER], tok[] = " \t\r\n", *point;
//*---------------------------------------------*    
	point = strtok((char*) command, tok);
//*---------------------------------------------*    
	while(point != NULL)
	{
		sscanf(point, "%s", temp);
//*---------------------------------------------*    		
		debug_parse_command(temp);
//*---------------------------------------------*    		
		command_list.push_back(new COMMAND(temp, strlen(temp)));
//*---------------------------------------------*    		
		point = strtok(NULL, tok);
	}
}
void COMMAND_LIST::check_pipe_vector(int vector_amount)
{
	debug_check_pipe_vector(vector_amount);
//*---------------------------------------------*    		
	while(pipe_vector.size() <= vector_amount)
		pipe_vector.push_back(new PIPE_LIST);
}
//***********************************************
void COMMAND_LIST::check_transfer_pipe(PIPE_TYPE pipe_type, int pipe_destination, bool handle_new_line)
{
	check_pipe_destination(pipe_vector[0], 0);
//***********************************************
	switch(pipe_type)
	{
		case PIPE_SINGLE:
		case PIPE_END:
			if(handle_new_line)
			{
				check_pipe_vector(pipe_destination);
				check_pipe_destination(pipe_vector[pipe_destination], 0);
				search_pipe_destination(pipe_vector[pipe_destination], 0);
			}
			break;
		case PIPE_FRONT:
		case PIPE_MIDDLE:
			check_pipe_destination(pipe_vector[0], pipe_destination);
			search_pipe_destination(pipe_vector[0], pipe_destination);
			break;
		default:
			break;
	}
}
//***********************************************
PARSE_RESULT COMMAND_LIST::parse_argv(PIPE_TYPE& pipe_type, int& exec_argc, int& remove_argc, int& pipe_destination, bool& handle_to_client, int& pipe_to_client, bool& handle_from_client, int& pipe_from_client, bool& handle_new_line, bool& handle_output_file, bool& handle_error)
{
	exec_argc = -1;
	remove_argc = -1;
	pipe_destination = 1;
	pipe_to_client = -1;
	pipe_from_client = -1;
	handle_to_client = false;
	handle_from_client = false;
	handle_new_line = false;
	handle_output_file = false;
	handle_error = false;
	bool handle_end_command = false;
//*---------------------------------------------*   	
	for(int i = 0; i < command_list.size() && !handle_end_command; i++)
		switch(command_list[i]->command[0])
		{
			case '|':
				if(strcmp(command_list[i]->command, "|") == 0)
					if(i == command_list.size() - 1)
					{
						error("pipe format '|'", false);
//*---------------------------------------------*   
						return PARSE_ERROR;
					}
					else
					{
						exec_argc = i;
						remove_argc = i + 1;
//*---------------------------------------------*   
						handle_end_command = true;
//*---------------------------------------------*   
						pipe_type = pipe_type == PIPE_SINGLE ? PIPE_FRONT : PIPE_MIDDLE;
					}
				else if(sscanf(command_list[i]->command, "|%d", &pipe_destination) == 1)
					if(i == command_list.size() - 1)
					{
						exec_argc = i;
						remove_argc = i + 1;
//*---------------------------------------------*   
						handle_new_line = true;
						handle_end_command = true;
//*---------------------------------------------*   
						pipe_type = pipe_type == PIPE_SINGLE ? PIPE_SINGLE : PIPE_END;
					}
					else
					{
						error("new line format '|N'", false);
//*---------------------------------------------*   
						return PARSE_ERROR;
					}
				else
				{
					error("pipe format '|'", false);
//*---------------------------------------------*   
					return PARSE_ERROR;
				}
				break;
			case '!':
				if(strcmp(command_list[i]->command, "!") == 0)
				{
					error("pipe format '!' ", false);
//*---------------------------------------------*   
					return PARSE_ERROR;
				}
				else if(sscanf(command_list[i]->command, "!%d", &pipe_destination) == 1)
					if(i == command_list.size() - 1)
					{
						exec_argc = i;
						remove_argc = i + 1;
//*---------------------------------------------*   
						handle_new_line = true;
						handle_end_command = true;
//*---------------------------------------------*   
						handle_error = true;
//*---------------------------------------------*   
						pipe_type = pipe_type == PIPE_SINGLE ? PIPE_SINGLE : PIPE_END;
					}
					else
					{
						error("new line format '!N'", false);
//*---------------------------------------------*   
						return PARSE_ERROR;
					}
				else
				{
					error("pipe format '!'", false);
//*---------------------------------------------*   
					return PARSE_ERROR;
				}
				break;
			case '>':
				if(strcmp(command_list[i]->command, ">") == 0)
					if(i + 1 == command_list.size() - 1)
					{
						exec_argc = i;
						remove_argc = i + 2;
//*---------------------------------------------*   
						handle_output_file = true;
						handle_end_command = true;
//*---------------------------------------------*   
						pipe_type = pipe_type == PIPE_SINGLE ? PIPE_SINGLE : PIPE_END;
					}
					else
					{
						error("output file format", false);
						return PARSE_ERROR;
					}
				else if(sscanf(command_list[i]->command, ">%d", &pipe_to_client) == 1)
				{
					if(exec_argc == -1)
						exec_argc = i;
					remove_argc = i + 1;
//*---------------------------------------------*   
					handle_to_client = true;
				}
				else
				{
					error("pipe format", false);
//*---------------------------------------------*   
					return PARSE_ERROR;
				}  
				break;
			case '<':
				if(sscanf(command_list[i]->command, "<%d", &pipe_from_client) == 1 && (pipe_type == PIPE_SINGLE || pipe_type == PIPE_FRONT))
				{
					if(exec_argc == -1)
						exec_argc = i;
					remove_argc = i + 1;
//*---------------------------------------------*   
					handle_from_client = true;
				}
				else
				{
					error("pipe format", false);
//*---------------------------------------------*   
					return PARSE_ERROR;
				}  
				break;
			default:
				break;
		}
	
	if((handle_to_client && handle_new_line) || (handle_to_client && handle_output_file) || (handle_output_file && handle_new_line))
		return PARSE_ERROR;
		
	switch(pipe_type)
	{
		case PIPE_SINGLE:
			break;
		case PIPE_FRONT:
			if(handle_new_line || handle_to_client || handle_output_file)
				return PARSE_ERROR;
			break;
		case PIPE_MIDDLE:
			if(handle_from_client || handle_new_line || handle_to_client || handle_output_file)
				return PARSE_ERROR;
			break;
		case PIPE_END:
			if(handle_from_client)
				return PARSE_ERROR;
			break;
	}	
	
	return PARSE_SUCCESS;
}
//***********************************************
void COMMAND_LIST::remove_current_command(int command_size)
{
	for(int i = 0; i < command_size; i++)
		delete_command_list_front();
}
//***********************************************
void COMMAND_LIST::exec_printenv()
{
	if(command_list.size() != 2)
	{
		error_exec_printenv_argc(command_list.size());
//*---------------------------------------------* 
		return;
	};	
//*---------------------------------------------* 
	int len;
	char *printenv;
//*---------------------------------------------* 
	printenv = environment_list.print_environment(command_list[1]->command);
//*---------------------------------------------* 					
	if(printenv != NULL)
	{		
		std::string printenv_string(printenv);
//*---------------------------------------------* 					
		printenv_string.append("\n");
//*---------------------------------------------* 					
		safety_write(socket_fd, printenv_string.c_str(), strlen(printenv_string.c_str()));
	}
	else
		error_exec_printenv();
}
//***********************************************
void COMMAND_LIST::exec_setenv()
{
	if(command_list.size() != 3)
	{
		error_exec_setenv_argc(command_list.size());
//*---------------------------------------------* 
		return;
	};	
//*---------------------------------------------* 
	environment_list.set_environment(command_list[1]->command, command_list[2]->command);
}
//***********************************************
EXEC_RESULT COMMAND_LIST::exec_command(PIPE_TYPE pipe_type, int pipe_destination, bool handle_to_client, int pipe_to_client, bool handle_from_client, int pipe_from_client, bool handle_new_line, bool handle_output_file, bool handle_error, int output_file_fd, char **argv, bool& exec_error)
{
	int from_client_fd, to_client_fd;
	exec_error = false; 
//*---------------------------------------------* 
	safety_signal(SIGCHLD, sig_chld_client);
//*---------------------------------------------* 
	if(handle_from_client)
		if(!(client_list->check_client_to_stdin(socket_fd, pipe_from_client, client_id, from_client_fd)))
			return EXEC_SUCCESS;
//*---------------------------------------------* 
	if(handle_to_client)
	{
		if(!(client_list->check_stdout_to_client(socket_fd, client_id, pipe_to_client, to_client_fd)))
			return EXEC_SUCCESS;
	}
//*---------------------------------------------* 
	int stat;	
	pid_t exec_pid = fork();	
//*---------------------------------------------* 
	if(exec_pid == 0)
	{
		switch(pipe_type)
		{
			case PIPE_SINGLE:
				if(handle_from_client)
					client_list->redirect_client_to_stdin(from_client_fd);
				else
					pipe_vector[0]->redirect_pipe_to_stdin();
//*---------------------------------------------* 
				if(handle_to_client)
					client_list->redirect_stdout_to_client(to_client_fd);
				else if(handle_output_file)
					redirect_stdout_to_output_file(output_file_fd, socket_fd, handle_error);
				else if(handle_new_line)
					redirect_stdout_to_middle(pipe_vector[pipe_destination], 0, socket_fd, handle_error);
				else
					redirect_stdout_to_socket(socket_fd, handle_error);
				break;
			case PIPE_FRONT:
				if(handle_from_client)
					client_list->redirect_client_to_stdin(from_client_fd);
				else
					pipe_vector[0]->redirect_pipe_to_stdin();
//*---------------------------------------------* 
				if(handle_to_client)
					return EXEC_FORBID;
				else if(handle_output_file)
					return EXEC_FORBID;
				else if(handle_new_line)
					return EXEC_FORBID;
				else
					redirect_stdout_to_middle(pipe_vector[0], pipe_destination, socket_fd, handle_error);
				break;
			case PIPE_MIDDLE:
				if(handle_from_client)
					return EXEC_FORBID;
				else
					pipe_vector[0]->redirect_pipe_to_stdin();
//*---------------------------------------------* 
				if(handle_to_client)
					return EXEC_FORBID;
				else if(handle_output_file)
					return EXEC_FORBID;
				else if(handle_new_line)
					return EXEC_FORBID;
				else
					redirect_stdout_to_middle(pipe_vector[0], pipe_destination, socket_fd, handle_error);
				break;
			case PIPE_END:
				if(handle_from_client)
					return EXEC_FORBID;
				else
					pipe_vector[0]->redirect_pipe_to_stdin();
//*---------------------------------------------* 
				if(handle_to_client)
					client_list->redirect_stdout_to_client(to_client_fd);
				else if(handle_output_file)
					redirect_stdout_to_output_file(output_file_fd, socket_fd, handle_error);
				else if(handle_new_line)
					redirect_stdout_to_middle(pipe_vector[pipe_destination], 0, socket_fd, handle_error);
				else
					redirect_stdout_to_socket(socket_fd, handle_error);
				break;
			default:
				break;
		}
//*---------------------------------------------* 
		extern char **environ;
		environ = environment_list.get_vector();
		execvp(command_list[0]->command, argv);
//*---------------------------------------------* 
		exit(EXEC_UNKNOWN);
	}
	else if(exec_pid > 0)
	{
		if(handle_from_client)
			;//client_list->close_client_to_stdin(from_client_fd);	
		else
			pipe_vector[0]->close_pipe_to_stdin();
//*---------------------------------------------* 
		switch(pipe_type)	
		{
			case PIPE_SINGLE:
				if(handle_to_client)
					;//client_list->close_stdout_to_client(to_client_fd);	
				else if(handle_output_file)
					close_stdout_to_output_file(output_file_fd);
				else if(handle_new_line)
					close_stdout_to_middle(pipe_vector[pipe_destination], 0);
				break;
			case PIPE_FRONT:
				close_stdout_to_middle(pipe_vector[0], pipe_destination);
				break;
			case PIPE_MIDDLE:
				close_stdout_to_middle(pipe_vector[0], pipe_destination);
				break;
			case PIPE_END:
				if(handle_to_client)
					;//client_list->close_stdout_to_client(to_client_fd);	
				else if(handle_output_file)
					close_stdout_to_output_file(output_file_fd);
				else if(handle_new_line)
					close_stdout_to_middle(pipe_vector[pipe_destination], 0);
				break;
		}		
	}
	else
	{
		error("exec fork", true);
		return EXEC_ERROR;
	}
//*---------------------------------------------* 	
	waitpid(-1, &stat, 0);
//*---------------------------------------------* 	
	if(handle_from_client)
		client_list->unlink_client_to_stdin(from_client_fd, pipe_from_client, client_id, command_string);	
	if(handle_to_client)
		client_list->broacast_stdout_to_client(to_client_fd, client_id, pipe_to_client, command_string);	
//*---------------------------------------------* 	
	if(WIFEXITED(stat) && WEXITSTATUS(stat) == EXEC_UNKNOWN)
		exec_error = true;
#if defined(DEBUG_COMMAND_EXEC_ERROR)
	if(WIFEXITED(stat))
		printf("normal termination,signalstatus=%d\n",WEXITSTATUS(stat));
	else
		printf("abnormal termination,signalstatus=%d\n",WTERMSIG(stat));
#endif
	return EXEC_SUCCESS;
}
//***********************************************
COMMAND_RETURN COMMAND_LIST::exec_pipe()
{
	PIPE_TYPE pipe_type = PIPE_SINGLE;
//*---------------------------------------------* 
	while(!command_list.empty())
	{
		if(strcmp(command_list[0]->command, "exit") == 0)
			return COMMAND_EXIT;	
//*---------------------------------------------* 	
		char **argv;	
		int command_argc, exec_argc, remove_argc, pipe_destination, pipe_to_client, pipe_from_client;
		bool handle_to_client, handle_from_client, handle_new_line, handle_output_file, handle_error;
		int output_file_fd;
//*---------------------------------------------* 			
		if(parse_argv(pipe_type, exec_argc, remove_argc, pipe_destination, handle_to_client, pipe_to_client, handle_from_client, pipe_from_client, handle_new_line, handle_output_file, handle_error) == PARSE_ERROR)
		{
			std::string error_fork("Input format error.\n");
//*---------------------------------------------* 
			safety_write(socket_fd, error_fork.c_str(), strlen(error_fork.c_str()));
			return COMMAND_ERROR;
		}
//*---------------------------------------------* 
		debug_parse_argv(pipe_type, exec_argc, remove_argc, pipe_destination, handle_to_client, pipe_to_client, handle_from_client, pipe_from_client, handle_new_line, handle_output_file, handle_error);
//*---------------------------------------------* 
		if(pipe_type == PIPE_SINGLE || pipe_type == PIPE_END)
			if(command_list.size() != remove_argc && remove_argc != -1)
			{
				printf("command should be empty\n");
				return COMMAND_ERROR;
			}
//*---------------------------------------------* 		
		if(handle_output_file)
		{
			FILE *ptr = fopen(command_list[remove_argc - 1]->command, "w");
//*---------------------------------------------* 
			if(ptr != NULL)
				output_file_fd = fileno(ptr);
			else
			{
				error("open file", false);
				return COMMAND_ERROR;
			}
		}
//*---------------------------------------------* 
		int argc = exec_argc == -1 ? command_list.size(): exec_argc;
//*---------------------------------------------* 
		argv = new char* [argc + 1];
		for(int i = 0; i < argc; i++)
			argv[i] = command_list[i]->command;
		argv[argc] = NULL;
//*---------------------------------------------* 
#if defined(DEBUG_COMMAND) && defined(DEBUG_COMMAND_PARSE_ARGV)
		for(int i = 0; i < argc; i++)
			printf("[TEST] argv[%d] len: %zu string: '%s'\n", i, strlen(argv[i]), argv[i]);
#endif
//*---------------------------------------------* 
		check_transfer_pipe(pipe_type, pipe_destination, handle_new_line);
//*---------------------------------------------* 
		bool exec_error; 
		EXEC_RESULT exec_result;
//*---------------------------------------------* 
		if((exec_result = exec_command(pipe_type, pipe_destination, handle_to_client, pipe_to_client, handle_from_client, pipe_from_client, handle_new_line, handle_output_file, handle_error, output_file_fd, argv, exec_error)) == EXEC_ERROR)
		{
			std::string error_fork("Exec fork not success.\n");
//*---------------------------------------------* 
			safety_write(socket_fd, error_fork.c_str(), strlen(error_fork.c_str()));
		}
		else if(exec_result == EXEC_FORBID)
		{
			std::string error_fork("Exec forbid.\n");
//*---------------------------------------------* 
			safety_write(socket_fd, error_fork.c_str(), strlen(error_fork.c_str()));
		}
//*---------------------------------------------* 
		if(exec_error)
		{
			std::string error_command("Unknown command: [");
			error_command.append(argv[0]);
			error_command.append("].\n");
//*---------------------------------------------* 
			safety_write(socket_fd, error_command.c_str(), strlen(error_command.c_str()));
		}
//*---------------------------------------------*
		delete(argv);
//*---------------------------------------------* 
		if(remove_argc == -1 || exec_error || exec_result == EXEC_FORBID)
			remove_current_command(command_list.size());
		else
			remove_current_command(remove_argc);
//*---------------------------------------------* 
		pipe_vector[0]->delete_pipe_list_front();
//*---------------------------------------------* 				
		pipe_type = PIPE_END;
	}
	return COMMAND_SUCCESS;	
}
//***********************************************
COMMAND_RETURN COMMAND_LIST::exec()
{
	if(command_list.empty())
		return COMMAND_SUCCESS;
//*---------------------------------------------* 
	if(strcmp(command_list[0]->command, "printenv") == 0)
	{
		exec_printenv();
//*---------------------------------------------* 					
		return COMMAND_SUCCESS;
	}
	else if(strcmp(command_list[0]->command, "setenv") == 0)
	{
		exec_setenv();
//*---------------------------------------------* 
		return COMMAND_SUCCESS;
	}
	else if(strcmp(command_list[0]->command, "who") == 0)
	{
		if(command_list.size() != 1)
		{
			error_exec_who_argc(command_list.size());
//*---------------------------------------------* 
			return COMMAND_SUCCESS;
		}
		else
			client_list->who(socket_fd, client_id);
//*---------------------------------------------* 
		return COMMAND_SUCCESS;
	}
	else if(strcmp(command_list[0]->command, "tell") == 0)
	{
		int transfer_id;
		if(command_list.size() < 2)
		{
			error_exec_tell_argc(command_list.size());
//*---------------------------------------------* 
			return COMMAND_SUCCESS;
		}
		else if(sscanf(command_list[1]->command, "%d", &transfer_id) != 1)
		{	
			error_exec_tell(command_list[1]->command);
//*---------------------------------------------* 
			return COMMAND_SUCCESS;
		}
		else
		{	
			std::string message("");
//*---------------------------------------------* t
			for(int i = 2;i < command_list.size(); i++)
			{	
				if(i != 2)
					message.append(" ");
				message.append(command_list[i]->command);
			}
//*---------------------------------------------* 
			client_list->tell(socket_fd, client_id, transfer_id, message.c_str());
		}
//*---------------------------------------------* 
		return COMMAND_SUCCESS;
	}
	else if(strcmp(command_list[0]->command, "yell") == 0)
	{
		if(command_list.size() < 1)
		{
			error_exec_yell_argc(command_list.size());
//*---------------------------------------------* 
			return COMMAND_SUCCESS;
		}
		else
		{
			std::string message("");
//*---------------------------------------------* 
			for(int i = 1;i < command_list.size(); i++)
			{	
				if(i != 1)
					message.append(" ");
				message.append(command_list[i]->command);
			}
//*---------------------------------------------* 
			client_list->yell(socket_fd, client_id, message.c_str());
		}
//*---------------------------------------------* 
		return COMMAND_SUCCESS;
	}
	else if(strcmp(command_list[0]->command, "name") == 0)
	{
		if(command_list.size() != 2)
		{
			error_exec_name_argc(command_list.size());
//*---------------------------------------------* 
			return COMMAND_SUCCESS;
		}
		else
			client_list->name(socket_fd, client_id, command_list[1]->command);
//*---------------------------------------------* 
		return COMMAND_SUCCESS;
	}
	else
		return exec_pipe();
}
//***********************************************
void COMMAND_LIST::delete_pipe_vector_front()
{
	debug_delete_pipe_vector_front(&pipe_vector);
//*---------------------------------------------*    		
	if(!pipe_vector.empty())
	{
		pipe_vector[0]->~PIPE_LIST();
		pipe_vector.pop_front();
	}
}
//***********************************************
void COMMAND_LIST::clear_pipe_vector()
{
	debug_clear_pipe_vector(this);
//*---------------------------------------------*    		
	while(!pipe_vector.empty())
		delete_pipe_vector_front();
}
//***********************************************
void COMMAND_LIST::delete_command_list_front()
{
	debug_delete_command_list_front(&command_list);
//*---------------------------------------------*    		
	if(!command_list.empty())
	{
		command_list[0]->~COMMAND();
		command_list.pop_front();
	}
}
//***********************************************
void COMMAND_LIST::clear_command_list()
{
	debug_clear_command_list(this);
//*---------------------------------------------*    		
	while(!command_list.empty())
		delete_command_list_front();
	delete_pipe_vector_front();
}
//***********************************************
void debug_command_constructor(COMMAND *addr, char *command)
{
#if defined(DEBUG_COMMAND) && defined(DEBUG_COMMAND_BUILDIN)
	printf("[SERV]|COMD| COMMAND constructor at %p len: %zu string: '%s'\n", (ssize_t*) addr, strlen(command), command);
#endif
}
//***********************************************
void debug_command_destructor(COMMAND *addr)
{
#if defined(DEBUG_COMMAND) && defined(DEBUG_COMMAND_BUILDIN)
	printf("[SERV]|COMD| COMMAND destructor at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_command_list_constructor(COMMAND_LIST *addr)
{
#if defined(DEBUG_COMMAND_LIST) && defined(DEBUG_COMMAND_LIST_BUILDIN)
	printf("[SERV]|CLST| COMMAND_LIST constructor at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_command_list_destructor(COMMAND_LIST *addr)
{
#if defined(DEBUG_COMMAND_LIST) && defined(DEBUG_COMMAND_LIST_BUILDIN)
	printf("[SERV]|CLST| COMMAND_LIST destructor at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_new_command()
{
#if defined(DEBUG_COMMAND) && defined(DEBUG_COMMAND_CREATE)
	printf("[SERV]|COMD| Create new COMMAND\n");
#endif
}
//***********************************************
void debug_parse_command(char *command)
{
#if defined(DEBUG_COMMAND) && defined(DEBUG_COMMAND_PARSE)
	printf("[SERV]|PARS| Parse new COMMAND len: %zu stirng: '%s'\n", strlen(command), command);
#endif
}
//***********************************************
void debug_check_pipe_vector(int vector_amount)
{
#if defined(DEBUG_PIPE_VECTOR) && defined(DEBUG_PIPE_VECTOR_EXIST)
	printf("[SERV]|PVTR| Check pipe_vector[%d]\n", vector_amount);
#endif
}
//***********************************************
void debug_delete_pipe_vector_front(void *addr)
{
#if defined(DEBUG_PIPE_VECTOR) && defined(DEBUG_PIPE_VECTOR_DELETE)
	printf("[SERV]|PVTR| Delete pipe_vector front at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_clear_pipe_vector(void *addr)
{
#if defined(DEBUG_PIPE_VECTOR) && defined(DEBUG_PIPE_VECTOR_CLEAR)
	printf("[SERV]|PVTR| Clear pipe_vector in COMMAND_LIST at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_delete_command_list_front(void *addr)
{
#if defined(DEBUG_COMMAND_LIST) && defined(DEBUG_COMMAND_LIST_DELETE)
	printf("[SERV]|CLST| Delete command_list front at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_clear_command_list(void *addr)
{
#if defined(DEBUG_COMMAND_LIST) && defined(DEBUG_COMMAND_LIST_CLEAR)
	printf("[SERV]|CLST| Clear command_list in COMMAND_LIST at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_parse_argv(PIPE_TYPE pipe_type, int exec_argc, int remove_argc, int pipe_destination, bool handle_to_client, int pipe_to_client, bool handle_from_client, int pipe_from_client, bool handle_new_line, bool handle_output_file, bool handle_error)
{
#if defined(DEBUG_COMMAND) && defined(DEBUG_COMMAND_PARSE_ARGV)
	printf("[SERV]|PAGV| Parse new COMMAND:\n");
	printf("[SERV]|PAGV| PIPE_TYPE: %s\n", pipe_type == PIPE_SINGLE ? "PIPE_SINGLE": pipe_type == PIPE_FRONT ? "PIPE_FRONT": pipe_type == PIPE_MIDDLE ? "PIPE_MIDDLE": "PIPE_END");
	printf("[SERV]|PAGV| exec_argc: %d\n", exec_argc);
	printf("[SERV]|PAGV| remove_argc: %d\n", remove_argc);
	printf("[SERV]|PAGV| pipe_destination: %d\n", pipe_destination);
	printf("[SERV]|PAGV| pipe_to_client: %d\n", pipe_to_client);
	printf("[SERV]|PAGV| pipe_from_client: %d\n", pipe_from_client);
	printf("[SERV]|PAGV| handle_to_client: %s\n", handle_to_client ? "TRUE" : "FALSE");
	printf("[SERV]|PAGV| handle_from_client: %s\n", handle_from_client ? "TRUE" : "FALSE");
	printf("[SERV]|PAGV| handle_output_file: %s\n", handle_output_file ? "TRUE" : "FALSE");
	printf("[SERV]|PAGV| handle_new_line: %s\n", handle_new_line ? "TRUE" : "FALSE");
	printf("[SERV]|PAGV| handle_error: %s\n", handle_error ? "TRUE" : "FALSE");
#endif
}
//***********************************************	
void error_exec_printenv_argc(int argc)
{
#if defined(ERROR_PRINTENV)
	printf("[ERROR] printenv argc: %d != 2\n", argc);
#endif
}
//***********************************************
void error_exec_printenv()
{
#if defined(ERROR_PRINTENV)
	printf("[ERROR] printenv exec error\n");
#endif
}
//***********************************************
void error_exec_setenv_argc(int argc)
{
#if defined(ERROR_SETENV)
	printf("[ERROR] setenv argc: %d != 3\n", argc);
#endif
}
//***********************************************
void error_exec_setenv()
{
#if defined(ERROR_SETENV)
	printf("[ERROR] setenv exec error\n");
#endif
}
//***********************************************
void error_exec_who_argc(int argc)
{
#if defined(ERROR_SETENV)
	printf("[ERROR] who argc: %d != 1\n", argc);
#endif
}
//***********************************************
void error_exec_tell_argc(int argc)
{
#if defined(ERROR_SETENV)
	printf("[ERROR] tell argc: %d < 2\n", argc);
#endif
}
//***********************************************
void error_exec_tell(char *parse)
{
#if defined(ERROR_SETENV)
	printf("[ERROR] tell argc: %s\n", parse);
#endif
}
//***********************************************
void error_exec_yell_argc(int argc)
{
#if defined(ERROR_SETENV)
	printf("[ERROR] yell argc: %d < 1\n", argc);
#endif
}
//***********************************************
void error_exec_name_argc(int argc)
{
#if defined(ERROR_SETENV)
	printf("[ERROR] name argc: %d != 2\n", argc);
#endif
}
//***********************************************
