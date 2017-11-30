#ifndef __SAFETY_FUNCTION__
#define __SAFETY_FUNCTION__
//***********************************************
#include "socket_header.h"
#include "pipe_list.h"
//***********************************************
int safety_port(int port);
int stafety_socket(int domain, int type, int protocol);
void safety_bind(int socket_fd, const struct sockaddr *addr, socklen_t addr_len);
void safety_listen(int socket_fd, int back_log);
void safety_signal(int signal_num, sighandler_t handler);
//***********************************************
void sig_chld_server(int signo);
//***********************************************
ssize_t safety_write(int fd, const void *buf, size_t N);
READLINE_RETURN safety_readline(int fd, char *vptr, size_t maxlen);
//***********************************************
int safety_shmget(key_t key, size_t size, int shmflg);
void* safety_shmat(int shmid, const void *shmaddr, int shmflg); 
void stafety_shmctl(int shmid, int cmd, struct shmid_ds *buf);
void stafety_shmdt(const void *shmaddr);  
//***********************************************
void debug_welcome();
void debug_readline(const char *readline);
void debug_client_connect(int client_id);
void debug_client_terminate(int client_id);
void debug_fork_information(int parent_pid, int child_pid);
void debug_sig_chld_server(int status);
//***********************************************
void debug_pipe_constructor(void *addr);
void debug_pipe_destructor(void *addr);
//***********************************************
void error(const char *error, bool handle_exit = true);
//***********************************************
void transfer_fd(int destination_fd, int source_fd);
//***********************************************
#endif
