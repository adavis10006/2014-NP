#ifndef __COMMAND_LIST__
#define __COMMAND_LIST__
//***********************************************
#include "socket_header.h"
#include "client_list.h"
#include "environment_list.h"
#include "pipe_list.h"
#include "semaphore.h"
//***********************************************
struct COMMAND;
//***********************************************
class COMMAND_LIST
{
public:	
	COMMAND_LIST(int client_id, int socket_fd);
	~COMMAND_LIST();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void new_command(const char *command);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void parse_command(const char *command);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void check_pipe_vector(int vector_amount);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void check_transfer_pipe(PIPE_TYPE pipe_type, int pipe_destination, bool handle_new_line);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	PARSE_RESULT parse_argv(PIPE_TYPE& pipe_type, int& exec_argc, int& remove_argc, int& pipe_destination, bool& handle_destination_client, int& destination_client, bool& handle_outset_client, int& outset_client, bool& handle_new_line, bool& handle_output_file, bool& handle_error);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void remove_current_command(int command_size);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void exec_printenv();
	void exec_setenv();
	EXEC_RESULT exec_command(PIPE_TYPE pipe_type, int pipe_destination, bool handle_destination_client, int destination_client, bool handle_outset_client, int outset_client, bool handle_new_line, bool handle_output_file, bool handle_error, int output_file_fd, char **argv, bool& exec_error);
	COMMAND_RETURN exec_pipe();
	COMMAND_RETURN exec();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void delete_pipe_vector_front();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void clear_pipe_vector();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void delete_command_list_front();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
	void clear_command_list();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
private:
	int client_id;
    int socket_fd;
	char *command_string;
	ENVIRONMENT_LIST environment_list;
	std::deque<PIPE_LIST*> pipe_vector;
	std::deque<COMMAND*> command_list;
};
//***********************************************
void debug_command_constructor(COMMAND *addr, char *command);
void debug_command_destructor(COMMAND *addr);
//***********************************************
void debug_command_list_constructor(COMMAND_LIST *addr);
void debug_command_list_destructor(COMMAND_LIST *addr);
//***********************************************
void debug_new_command();
void debug_parse_command(char *command);
//***********************************************
void debug_check_pipe_vector(int vector_amount);
//***********************************************
void debug_delete_pipe_vector_front(void *addr);
void debug_clear_pipe_vector(void *addr);
//***********************************************
void debug_delete_command_list_front(void *addr);
void debug_clear_command_list(void *addr);
//***********************************************
void debug_parse_argv(PIPE_TYPE pipe_type, int exec_argc, int remove_argc, int pipe_destination, bool handle_destination_client, int destination_client, bool handle_outset_client, int outset_client, bool handle_new_line, bool handle_output_file, bool handle_error);	
//***********************************************
void error_exec_printenv_argc(int argc);
void error_exec_printenv();
//***********************************************
void error_exec_setenv_argc(int argc);
void error_exec_setenv();
//***********************************************
void error_exec_who_argc(int argc);
void error_exec_tell_argc(int argc);
void error_exec_tell(char *parse);
void error_exec_yell_argc(int argc);
void error_exec_name_argc(int argc);
//***********************************************
#endif
