#include "pipe_list.h"
//***********************************************
struct PIPE
{
	int pipe_fd[2];
	int middle_fd[2];
	bool create_pipe;
	bool create_middle;
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	PIPE()
	{
		debug_pipe_constructor(this);
//*---------------------------------------------*
		create_pipe = create_middle = false;
	}
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	~PIPE()
	{
		debug_pipe_destructor(this);
//*---------------------------------------------*
		if(create_pipe)
		{
			close(pipe_fd[PIPE_WRITE]);
			close(pipe_fd[PIPE_READ]);
		}
//*---------------------------------------------*
		if(create_middle)
		{
			close(middle_fd[PIPE_WRITE]);
			close(middle_fd[PIPE_READ]);
		}
	}
};
//***********************************************
void redirect_stdout_to_middle(PIPE_LIST *pipe_list, int destination, int socket_fd, bool handle_error)
{
	debug_redirect_stdout_to_middle(pipe_list, destination);
//*---------------------------------------------*
	close(pipe_list->pipe_list[destination]->middle_fd[PIPE_READ]);
//*---------------------------------------------*
	dup2(pipe_list->pipe_list[destination]->middle_fd[PIPE_WRITE], STDOUT_FILENO);
//*---------------------------------------------*
	if(handle_error)
		dup2(pipe_list->pipe_list[destination]->middle_fd[PIPE_WRITE], STDERR_FILENO);
	else
		dup2(socket_fd, STDERR_FILENO);
//*---------------------------------------------*
	close(pipe_list->pipe_list[destination]->middle_fd[PIPE_WRITE]);
	close(socket_fd);
}
//***********************************************
void close_stdout_to_middle(PIPE_LIST *pipe_list, int destination)
{
	debug_close_stdout_to_middle(pipe_list, destination);
//*---------------------------------------------*
	close(pipe_list->pipe_list[destination]->middle_fd[PIPE_WRITE]);
//*---------------------------------------------*
	transfer_fd(pipe_list->pipe_list[destination]->pipe_fd[PIPE_WRITE], pipe_list->pipe_list[destination]->middle_fd[PIPE_READ]);
//*---------------------------------------------*
	close(pipe_list->pipe_list[destination]->middle_fd[PIPE_READ]);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	pipe_list->pipe_list[destination]->create_middle = false;
}
//***********************************************
void check_pipe_destination(PIPE_LIST *addr, int destination)
{
	debug_check_pipe_destination(addr, destination);
//*---------------------------------------------*			
	while(addr->pipe_list.size() <= destination)
		addr->pipe_list.push_back(new PIPE);
//*---------------------------------------------*
	//search_pipe_destination(addr, destination);
}
//***********************************************
void search_pipe_destination(PIPE_LIST *addr, int destination)
{
	debug_search_pipe_destination(addr, destination);
//*---------------------------------------------*
	if(!(addr->pipe_list[destination]->create_pipe))
	{
		debug_pipe_set_pipe(addr->pipe_list[destination]);
//*---------------------------------------------*
		addr->pipe_list[destination]->create_pipe = true;
		pipe(addr->pipe_list[destination]->pipe_fd);
	}
//*---------------------------------------------*
	if(!(addr->pipe_list[destination]->create_middle))
	{
		debug_pipe_set_middle(addr->pipe_list[destination]);
//*---------------------------------------------*
		addr->pipe_list[destination]->create_middle = true;
		pipe(addr->pipe_list[destination]->middle_fd);
	}
}
//***********************************************
PIPE_LIST::~PIPE_LIST()
{
	debug_pipe_list_destructor(this);
//*---------------------------------------------*
	clear_pipe_list();
}
//***********************************************
void PIPE_LIST::redirect_pipe_to_stdin()
{
	if(pipe_list[0]->create_pipe)
	{
		debug_redirect_pipe_to_stdin(pipe_list[0]);
//*---------------------------------------------*
		close(pipe_list[0]->pipe_fd[PIPE_WRITE]);
		dup2(pipe_list[0]->pipe_fd[PIPE_READ], STDIN_FILENO);
		close(pipe_list[0]->pipe_fd[PIPE_READ]);
	}
}
//***********************************************
void PIPE_LIST::close_pipe_to_stdin()
{
	if(pipe_list[0]->create_pipe)
	{
		debug_close_pipe_to_stdin(pipe_list[0]);
//*---------------------------------------------*
		close(pipe_list[0]->pipe_fd[PIPE_READ]);
		close(pipe_list[0]->pipe_fd[PIPE_WRITE]);
	}
}
//***********************************************
void PIPE_LIST::delete_pipe_list_front()
{
	debug_delete_pipe_list_front(&pipe_list);
//*---------------------------------------------*
	if(!pipe_list.empty())
	{
		pipe_list.front()->~PIPE();
		pipe_list.pop_front();
	}
}
//***********************************************
void PIPE_LIST::clear_pipe_list()
{
	debug_clear_pipe_list(this);
//*---------------------------------------------*
	while(!pipe_list.empty())
		delete_pipe_list_front();
}
//***********************************************
int PIPE_LIST::pipe_list_size()
{
	return pipe_list.size();
}
//***********************************************
void PIPE_LIST::pipe_list_show()
{
	printf("[SERV]|PLST| Start to show pipe list at %p\n", this);
	for(int i = 0; i < pipe_list.size(); i++)
		printf("[SERV]|PLST| No.%d pipe: %s middle: %s\n", i, pipe_list[ i ]->create_pipe ? "True" : "False", pipe_list[ i ]->create_middle ? "True" : "False");
	printf("[SERV]|PLST| Start to show pipe list\n");
}
//***********************************************
void redirect_stdout_to_socket(int socket_fd, bool handle_error)
{
	debug_redirect_stdout_to_socket(socket_fd);
//*---------------------------------------------*
	dup2(socket_fd, STDOUT_FILENO);
//*---------------------------------------------*
	//if(handle_error)
	dup2(socket_fd, STDERR_FILENO);	
//*---------------------------------------------*
	close(socket_fd);
}
//***********************************************
void redirect_stdout_to_output_file(int output_file_fd, int socket_fd, bool handle_error)
{
	debug_redirect_stdout_to_output_file(output_file_fd, socket_fd);
//*---------------------------------------------*
	dup2(output_file_fd, STDOUT_FILENO);
//*---------------------------------------------*
	if(handle_error)
		dup2(output_file_fd, STDERR_FILENO);
	else
		dup2(socket_fd, STDERR_FILENO);
//*---------------------------------------------*
	close(output_file_fd);
	close(socket_fd);
}
//***********************************************
void close_stdout_to_socket(int socket_fd)
{
	debug_close_stdout_to_socket(socket_fd);
//*---------------------------------------------*
	//do nothing
	//close(socket_fd);
}
//***********************************************
void close_stdout_to_output_file(int output_file_fd)
{
	debug_close_stdout_to_output_file(output_file_fd);
//*---------------------------------------------*
	close(output_file_fd);
}
void debug_pipe_constructor(void *addr)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_BUILDIN)
	printf("[SERV]|PIPE| PIPE constructor at %p\n", (int*) addr);
#endif
}
//***********************************************
void debug_pipe_destructor(void *addr)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_BUILDIN)
	printf("[SERV]|PIPE| PIPE destructor at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_check_pipe_destination(PIPE_LIST *addr, int destination)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_EXIST)
	printf("[SERV]|PIPE| Check pipe_list[%d] at %p\n", destination, addr);
#endif
}
//***********************************************
void debug_search_pipe_destination(PIPE_LIST *addr, int destination)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_EXIST)
	printf("[SERV]|PIPE| Search PIPE at %p\n", addr->pipe_list[destination]);
#endif
}
//***********************************************
void debug_pipe_set_pipe(void *addr)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_SET)
	printf("[SERV]|PIPE| Set pipe at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_pipe_set_middle(void *addr)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_SET)
	printf("[SERV]|PIPE| Set middle at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_redirect_pipe_to_stdin(void *addr)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_REDIRECT)
	printf("[SERV]|PIPE| Redirect pipe to stdin at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_redirect_stdout_to_socket(int socket_fd)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_REDIRECT)
	printf("[SERV]|PIPE| Redirect stdout to socket %d\n", socket_fd);
#endif
}
//***********************************************
void debug_redirect_stdout_to_output_file(int output_file_fd, int socket_fd)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_REDIRECT)
	printf("[SERV]|PIPE| Redirect stdout to output file %d socket %d\n", output_file_fd, socket_fd);
#endif
}
//***********************************************
void debug_redirect_stdout_to_middle(PIPE_LIST *addr, int destination)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_REDIRECT)
	printf("[SERV]|PIPE| Redirect stdout to middle at %p\n", addr->pipe_list[destination]);
#endif
}
//***********************************************
void debug_close_pipe_to_stdin(void *addr)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_CLOSE)
	printf("[SERV]|PIPE| Close pipe to stdin at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_close_stdout_to_socket(int socket_fd)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_CLOSE)
	printf("[SERV]|PIPE| Close stdout to socket %d\n", socket_fd);
#endif
}
//***********************************************
void debug_close_stdout_to_output_file(int output_file_fd)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_CLOSE)
	printf("[SERV]|PIPE| Close stdout to output_file %d\n", output_file_fd);
#endif
}
//***********************************************
void debug_close_stdout_to_middle(PIPE_LIST *addr, int destination)
{
#if defined(DEBUG_PIPE) && defined(DEBUG_PIPE_CLOSE)
	printf("[SERV]|PIPE| Close stdout to middle at %p\n", addr->pipe_list[destination]);
#endif
}
//***********************************************
void debug_pipe_list_destructor(void *addr)
{
#if defined(DEBUG_PIPE_LIST) && defined(DEBUG_PIPE_LIST_BUILDIN)
	printf("[SERV]|PLST| PIPE_LIST destructor at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_delete_pipe_list_front(void *addr)
{
#if defined(DEBUG_PIPE_LIST) && defined(DEBUG_PIPE_LIST_DELETE)
	printf("[SERV]|PLST| Delete pipe_list front at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
void debug_clear_pipe_list(void *addr)
{
#if defined(DEBUG_PIPE_LIST) && defined(DEBUG_PIPE_LIST_CLEAR)
	printf("[SERV]|PLST| Clear pipe_list in PIPE_LIST at %p\n", (ssize_t*) addr);
#endif
}
//***********************************************
