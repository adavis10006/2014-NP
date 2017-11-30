#include "command_list.h"
//***********************************************
struct COMMAND
{
	char *command;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	COMMAND(char *buffer, int len)
	{
		command = new char[len + 1];
//-----------------------------------------------    
		strncpy(command, buffer, len);
		command[len] = '\0';
//-----------------------------------------------    
		debug_command_constructor(this, command);
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~COMMAND()
	{
		debug_command_destructor(this);
//-----------------------------------------------    		
		delete(command);
	}
};
//***********************************************
COMMAND_LIST::COMMAND_LIST(int socket_fd): socket_fd(socket_fd)
{
	debug_command_list_constructor(this);
}
//***********************************************
COMMAND_LIST::~COMMAND_LIST()
{
	debug_command_list_destructor(this);
//-----------------------------------------------    		
	clear_pipe_vector();
	clear_command_list();
}
//***********************************************
void COMMAND_LIST::new_command(const char *command)
{
	debug_new_command();
//-----------------------------------------------    		
	parse_command(command);
	check_pipe_vector(0);
}
//***********************************************
void COMMAND_LIST::parse_command(const char* command)
{
	char temp[SINGLE_COMMAND_MAXBUFFER], tok[] = " \t\r\n", *point;
//-----------------------------------------------    
	point = strtok((char*) command, tok);
//-----------------------------------------------    
	while(point != NULL)
	{
		sscanf(point, "%s", temp);
//-----------------------------------------------    		
		debug_parse_command(temp);
//-----------------------------------------------    		
		command_list.push_back(new COMMAND(temp, strlen(temp)));
//-----------------------------------------------    		
		point = strtok(NULL, tok);
	}
}
void COMMAND_LIST::check_pipe_vector(int vector_amount)
{
	debug_check_pipe_vector(vector_amount);
//-----------------------------------------------    		
	while(pipe_vector.size() <= vector_amount)
		pipe_vector.push_back(new PIPE_LIST);
}
//***********************************************
void COMMAND_LIST::check_transfer_pipe(PIPE_TYPE pipe_type, int pipe_destination)
{
	check_pipe_destination(pipe_vector[0], 0);
//***********************************************
	switch(pipe_type)
	{
		case PIPE_SINGLE:
		case PIPE_END:
		case PIPE_OUTPUT_FILE:
			break;
		case PIPE_FRONT:
		case PIPE_MIDDLE:
			check_pipe_destination(pipe_vector[0], pipe_destination);
			search_pipe_destination(pipe_vector[0], pipe_destination);
			break;
		case PIPE_NEW_LINE:
			check_pipe_vector(pipe_destination);
			check_pipe_destination(pipe_vector[pipe_destination], 0);
			search_pipe_destination(pipe_vector[pipe_destination], 0);
			break;
		default:
			break;
	}
}
//***********************************************
PARSE_RESULT COMMAND_LIST::parse_argv(PIPE_TYPE& pipe_type, int& exec_argc, int& pipe_destination, bool& handle_error)
{
	exec_argc = 0;
	pipe_destination = 1;
	handle_error = false;
//-----------------------------------------------   	
	for(int i = 0; i < command_list.size() && exec_argc == 0; i++)
		switch(command_list[i]->command[0])
		{
			case '|':
				if(strcmp(command_list[i]->command, "|") == 0)
					if(i == command_list.size() - 1)
					{
						error("pipe format '|'", false);
//-----------------------------------------------   
						return PARSE_ERROR;
					}
					else
					{
						exec_argc = i;
//-----------------------------------------------   
						pipe_type = pipe_type == PIPE_SINGLE ? PIPE_FRONT : PIPE_MIDDLE;
					}
				else if(sscanf(command_list[i]->command, "|%d", &pipe_destination) == 1)
					if(i == command_list.size() - 1)
					{
						exec_argc = i;
//-----------------------------------------------   
						pipe_type = PIPE_NEW_LINE;
					}
					else
					{
						error("new line format '|N'", false);
//-----------------------------------------------   
						return PARSE_ERROR;
					}
				else
				{
					error("pipe format '|'", false);
//-----------------------------------------------   
					return PARSE_ERROR;
				}
				break;
			case '!':
				if(strcmp(command_list[i]->command, "!") == 0)
				{
					//exec_argc = i;
//-----------------------------------------------   
					//handle_error = true;
//-----------------------------------------------   
					//pipe_type = pipe_type == PIPE_SINGLE ? PIPE_FRONT : PIPE_MIDDLE;
//-----------------------------------------------   
					error("pipe format '!' ", false);
//-----------------------------------------------   
					return PARSE_ERROR;
				}
				else if(sscanf(command_list[i]->command, "!%d", &pipe_destination) == 1)
					if(i == command_list.size() - 1)
					{
						exec_argc = i;
//-----------------------------------------------   
						handle_error = true;
//-----------------------------------------------   
						pipe_type = PIPE_NEW_LINE;
					}
					else
					{
						error("new line format '!N'", false);
//-----------------------------------------------   
						return PARSE_ERROR;
					}
				else
				{
					error("pipe format '!'", false);
//-----------------------------------------------   
					return PARSE_ERROR;
				}
				break;
			case '>':
				if(strcmp(command_list[i]->command, ">") == 0)
					if(i + 1 == command_list.size() - 1)
					{
						exec_argc = i;
//-----------------------------------------------   
						pipe_type = PIPE_OUTPUT_FILE;
					}
					else
					{
						error("output file format", false);
						return PARSE_ERROR;
					}
				else
				{
					error("pipe format", false);
//-----------------------------------------------   
					return PARSE_ERROR;
				}  
				break;
			default:
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
//----------------------------------------------- 
		return;
	};	
//----------------------------------------------- 
	int len;
	char *printenv;
//----------------------------------------------- 
	printenv = getenv(command_list[1]->command);
	len = strlen(printenv);
//----------------------------------------------- 
	std::string printenv_string(command_list[1]->command);
//----------------------------------------------- 					
	printenv_string.append("=");
	printenv_string.append(printenv, len);
	printenv_string.append("\n");
//----------------------------------------------- 					
	if(printenv != NULL)
		safety_write( socket_fd, printenv_string.c_str(), strlen(printenv_string.c_str()) );
	else
		error_exec_printenv();
}
//***********************************************
void COMMAND_LIST::exec_setenv()
{
	if(command_list.size() != 3)
	{
		error_exec_setenv_argc(command_list.size());
//----------------------------------------------- 
		return;
	};	
//----------------------------------------------- 
	if(setenv(command_list[1]->command, command_list[2]->command, 1) != 0)
		error_exec_setenv();
}
//***********************************************
EXEC_RESULT COMMAND_LIST::exec_command(PIPE_TYPE pipe_type, int pipe_destination, bool handle_error, int output_file_fd, char **argv, bool *exec_error)
{
	*exec_error = false;
//----------------------------------------------- 
	int stat;	
	pid_t exec_pid = fork();	
//----------------------------------------------- 
	if(exec_pid == 0)
	{
		pipe_vector[0]->redirect_pipe_to_stdin();
//----------------------------------------------- 
		switch(pipe_type)
		{
			case PIPE_SINGLE:
			case PIPE_END:
				redirect_stdout_to_socket(socket_fd, handle_error);
				break;
			case PIPE_OUTPUT_FILE:
				redirect_stdout_to_output_file(output_file_fd, socket_fd, handle_error);
				break;
			case PIPE_FRONT:
			case PIPE_MIDDLE:
				redirect_stdout_to_middle(pipe_vector[0], pipe_destination, socket_fd, handle_error);
				break;
			case PIPE_NEW_LINE:
				redirect_stdout_to_middle(pipe_vector[pipe_destination], 0, socket_fd, handle_error);
				break;
			default:
				break;
		}
//----------------------------------------------- 
		if(execvp(command_list[0]->command, argv) == -1)
			*exec_error = true;//printf("Unknown command: [%s]\n", argv[0]);
		
		exit(EXIT_FAILURE);
	}
	else if(exec_pid > 0)
	{
		pipe_vector[0]->close_pipe_to_stdin();
//----------------------------------------------- 
		switch(pipe_type)	
		{
			case PIPE_SINGLE:
			case PIPE_END:
				//close_stdout_to_socket(socket_fd);
				break;
			case PIPE_OUTPUT_FILE:
				//close_stdout_to_socket(output_file_fd);
				break;
			case PIPE_FRONT:
			case PIPE_MIDDLE:
				close_stdout_to_middle(pipe_vector[0], pipe_destination);
				break;
			case PIPE_NEW_LINE:
				close_stdout_to_middle(pipe_vector[pipe_destination], 0);
				break;
			default:
				break;
		}		
	}
	else
	{
		error("exec fork", true);
		return EXEC_ERROR;
	}
//----------------------------------------------- 
	waitpid(-1, &stat, 0);
}
//***********************************************
COMMAND_RETURN COMMAND_LIST::exec_pipe()
{
	PIPE_TYPE pipe_type = PIPE_SINGLE;
//----------------------------------------------- 
	while(!command_list.empty())
	{
		if(strcmp(command_list[0]->command, "exit") == 0)
			return COMMAND_EXIT;	
//----------------------------------------------- 	
		char **argv;	
		int command_argc, exec_argc, pipe_destination, output_destination;
		bool handle_error;
		int output_file_fd;
//----------------------------------------------- 			
		if(parse_argv(pipe_type, exec_argc, pipe_destination, handle_error) == PARSE_ERROR)
		{
			std::string error_fork("Input format error.\n");
//----------------------------------------------- 
			safety_write(socket_fd, error_fork.c_str(), strlen(error_fork.c_str()));
			return COMMAND_ERROR;
		}
//----------------------------------------------- 
		debug_parse_argv(pipe_type, exec_argc, pipe_destination, handle_error);
//----------------------------------------------- 
		if(pipe_type == PIPE_OUTPUT_FILE)
		{
			FILE *ptr = fopen(command_list[exec_argc + 1]->command, "w");
//----------------------------------------------- 
			if(ptr != NULL)
				output_file_fd = fileno(ptr);
			else
			{
				error("open file", false);
				return COMMAND_ERROR;
			}
		}
//----------------------------------------------- 
		int argc = exec_argc == 0 ? command_list.size(): exec_argc;
//----------------------------------------------- 
		argv = new char* [argc + 1];
		for(int i = 0; i < argc; i++)
			argv[i] = command_list[i]->command;
		argv[argc] = NULL;
//----------------------------------------------- 
#if defined(DEBUG_COMMAND) && defined(DEBUG_COMMAND_PARSE_ARGV)
		for(int i = 0; i < argc; i++)
			printf("[TEST] argv[%d] len: %zu string: '%s'\n", i, strlen(argv[i]), argv[i]);
#endif
//----------------------------------------------- 
		check_transfer_pipe(pipe_type, pipe_destination);
//----------------------------------------------- 
		int shmid = safety_shmget(IPC_PRIVATE, sizeof(bool), IPC_CREAT | SHM_MODE);
		bool *exec_error = (bool*) safety_shmat(shmid, NULL, 0); 
//-----------------------------------------------
#if defined(DEBUG_COMMAND_SHARE_MEMORY)
		printf("[SERV]|SHME| create share memory at %p\n", exec_error);
#endif
//----------------------------------------------- 		
		if(exec_error == NULL)
		{
			std::string error_fork("Create share memory  not success.\n");
//----------------------------------------------- 
			safety_write(socket_fd, error_fork.c_str(), strlen(error_fork.c_str()));
		}
//----------------------------------------------- 
		if(exec_command(pipe_type, pipe_destination, handle_error, output_file_fd, argv, exec_error) == EXEC_ERROR)
		{
			std::string error_fork("Exec fork not success.\n");
//----------------------------------------------- 
			safety_write(socket_fd, error_fork.c_str(), strlen(error_fork.c_str()));
		}
//----------------------------------------------- 
		if(*exec_error)
		{
			std::string error_command("Unknown command: [");
			error_command.append(argv[0]);
			error_command.append("].\n");
//----------------------------------------------- 
			safety_write(socket_fd, error_command.c_str(), strlen(error_command.c_str()));
		}
//----------------------------------------------- 
		delete(argv);
//----------------------------------------------- 
		if(pipe_type == PIPE_OUTPUT_FILE || pipe_type == PIPE_NEW_LINE || exec_argc == 0 || *exec_error)
			remove_current_command(command_list.size());
		else if(exec_argc != 0)
			remove_current_command(exec_argc + 1);
//----------------------------------------------- 		
		pipe_vector[0]->delete_pipe_list_front();
//----------------------------------------------- 				
		pipe_type = PIPE_END;
//----------------------------------------------- 	
		stafety_shmctl(shmid,IPC_RMID, NULL);
		stafety_shmdt(exec_error);  
//----------------------------------------------- 					
		//if(pipe_type == PIPE_OUTPUT_FILE)
			//close(output_file_fd);
	}
	return COMMAND_SUCCESS;	
}
//***********************************************
COMMAND_RETURN COMMAND_LIST::exec()
{
	if(command_list.empty())
		return COMMAND_SUCCESS;
//----------------------------------------------- 
	if(strcmp(command_list[0]->command, "printenv") == 0)
	{
		exec_printenv();
//----------------------------------------------- 					
		return COMMAND_SUCCESS;
	}
	else if(strcmp(command_list[0]->command, "setenv") == 0)
	{
		exec_setenv();
//----------------------------------------------- 
		return COMMAND_SUCCESS;
	}
	else
		return exec_pipe();
}
//***********************************************
void COMMAND_LIST::delete_pipe_vector_front()
{
	debug_delete_pipe_vector_front(&pipe_vector);
//-----------------------------------------------    		
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
//-----------------------------------------------    		
	while(!pipe_vector.empty())
		delete_pipe_vector_front();
}
//***********************************************
void COMMAND_LIST::delete_command_list_front()
{
	debug_delete_command_list_front(&command_list);
//-----------------------------------------------    		
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
//-----------------------------------------------    		
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
void debug_parse_argv(PIPE_TYPE pipe_type, int exec_argc, int pipe_destination, bool handle_error)			
{
#if defined(DEBUG_COMMAND) && defined(DEBUG_COMMAND_PARSE_ARGV)
	printf("[SERV]|PAGV| Parse new COMMAND:\n");
	printf("[SERV]|PAGV| PIPE_TYPE: %s\n", pipe_type == PIPE_SINGLE ? "PIPE_SINGLE": pipe_type == PIPE_FRONT ? "PIPE_FRONT": pipe_type == PIPE_MIDDLE ? "PIPE_MIDDLE": pipe_type == PIPE_END ? "PIPE_END": pipe_type == PIPE_NEW_LINE ? "PIPE_NEW_LINE": "PIPE_OUTPUT_FILE");
	printf("[SERV]|PAGV| exec_argc: %d\n", exec_argc);
	printf("[SERV]|PAGV| pipe_destination: %d\n", pipe_destination);
	printf("[SERV]|PAGV| handle_error: %s\n", handle_error ? "TRUE" : "FALSE");
	printf("[SERV]|PAGV| handle_output_file: %s\n", pipe_type == PIPE_OUTPUT_FILE ? "TRUE" : "FALSE");
	printf("[SERV]|PAGV| handle_new_line: %s\n", pipe_type == PIPE_NEW_LINE ? "TRUE" : "FALSE");
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