#ifndef __PIPE_LIST__
#define __PIPE_LIST__
//***********************************************
#include "socket_header.h"
#include "safety_function.h"
//***********************************************
struct PIPE;
//***********************************************
class PIPE_LIST
{
public:
	~PIPE_LIST();
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	friend void check_pipe_destination(PIPE_LIST *addr, int destination);
	friend void search_pipe_destination(PIPE_LIST *addr, int destination);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void redirect_pipe_to_stdin();
	friend void redirect_stdout_to_middle(PIPE_LIST *pipe_list, int destination, int socket_fd, bool handle_error);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void close_pipe_to_stdin();
	friend void close_stdout_to_middle(PIPE_LIST *pipe_list, int destination);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void delete_pipe_list_front();
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void clear_pipe_list();
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int pipe_list_size();
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void pipe_list_show();
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	friend void debug_check_pipe_destination(PIPE_LIST *addr, int destination);
	friend void debug_search_pipe_destination(PIPE_LIST *addr, int destination);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	friend void debug_redirect_stdout_to_middle(PIPE_LIST *addr, int destination);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	friend void debug_close_stdout_to_middle(PIPE_LIST *addr, int destination);
private:
	std::deque<PIPE*> pipe_list;
};
//***********************************************
void check_pipe_destination(PIPE_LIST *addr, int destination);
void search_pipe_destination(PIPE_LIST *addr, int destination);
//***********************************************
void redirect_stdout_to_socket(int socket_fd, bool handle_error);
void redirect_stdout_to_output_file(int output_file_fd, int socket_fd, bool handle_error);
void redirect_stdout_to_middle(PIPE_LIST *pipe_list, int destination, int socket_fd, bool handle_error);
//***********************************************
void close_stdout_to_socket(int socket_fd);
void close_stdout_to_middle(PIPE_LIST *pipe_list, int destination);
//***********************************************
void debug_check_pipe_destination(PIPE_LIST *addr, int destination);
void debug_search_pipe_destination(PIPE_LIST *addr, int destination);
//***********************************************
void debug_pipe_set_pipe(void *addr);
void debug_pipe_set_middle(void *addr);
//***********************************************
void debug_redirect_pipe_to_stdin(void *addr);
void debug_redirect_stdout_to_socket(int socket_fd);
void debug_redirect_stdout_to_output_file(int output_file_fd, int socket_fd);
void debug_redirect_stdout_to_middle(PIPE_LIST *addr, int destination);
//***********************************************
void debug_close_pipe_to_stdin(void *addr);
void debug_close_stdout_to_socket(int socket_fd);
void debug_close_stdout_to_middle(PIPE_LIST *addr, int destination);
//***********************************************
void debug_pipe_list_destructor(void *addr);
//***********************************************
void debug_delete_pipe_list_front(void *addr);
void debug_clear_pipe_list(void *addr);
//***********************************************
#endif
