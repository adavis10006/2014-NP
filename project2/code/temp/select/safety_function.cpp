#include "safety_function.h"
//***********************************************
int safety_port(int port)
{
	if(port <= 65536 && port >= 1024)
		return port;
	else
		error("port");
}
//***********************************************
int stafety_socket(int domain, int type, int protocol)
{
	int socket_fd;
//*---------------------------------------------*    
	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		error("socket");
	else
		return socket_fd;
}
//***********************************************
void safety_bind(int socket_fd, const struct sockaddr *addr, socklen_t addr_len)
{
	if(bind(socket_fd, addr, addr_len) == -1)
		error("bind");
}
//***********************************************	
void safety_listen(int socket_fd, int back_log)
{
	if(listen(socket_fd, back_log) == -1)
		error("listen");
}
//***********************************************	
void safety_signal(int signal_num, sig_t handler)
{	
	if(signal(signal_num, handler) == SIG_ERR)
		error("signal");
}
//***********************************************	
void sig_chld_server(int signo)
{
    pid_t pid;
    int stat;
//*---------------------------------------------*    
	debug_sig_chld_server();
//*---------------------------------------------*    
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0);
}
//***********************************************	
void sig_chld_client(int signo)
{
    pid_t pid;
    int stat;
//*---------------------------------------------*    
	debug_sig_chld_client();
//*---------------------------------------------*    
    while((pid = waitpid(getpid(), &stat, WNOHANG)) > 0);
}
//***********************************************
ssize_t safety_write(int fd, const void *buf, size_t N)
{
	ssize_t len = write(fd, buf, N);
//*---------------------------------------------*    
	/*if(len == -1)
		error("write");
	else*/
	return len;
}
//***********************************************
READLINE_RETURN safety_readline(int fd, char *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char c, *ptr;
//*---------------------------------------------*    
	ptr = vptr;
//*---------------------------------------------*    
	for(n = 1; n < maxlen; n++)
	{
again:
		if((rc = read(fd, &c, 1)) == 1) 
			if(c == '\n')
			{
				*ptr = 0;
				return READLINE_FINISH;
			}
			else
				*ptr++ = c;
		else if(rc == 0) 
		{
			*ptr = 0;
			return READLINE_EOF;
		} 
		else 
		{
			if(errno == EINTR)
				goto again;
			return READLINE_ERROR;
		}
	}
//*---------------------------------------------*    
	*ptr = 0;
	return READLINE_UNDONE;
}
//***********************************************
int safety_shmget(key_t key, size_t size, int shmflg)
{
	int shmid;
//*---------------------------------------------*    
	if((shmid = shmget(key, size, shmflg)) == -1)
		error("shmget", SHM_ERROR);	
	else
		return shmid;
}
//***********************************************
void* safety_shmat(int shmid, const void *shmaddr, int shmflg)
{
	void *addr;
	if((addr = shmat(shmid, shmaddr, shmflg)) == (void*) -1)
	{	
		error("shmat", SHM_ERROR);
		return NULL;
	}
	else
		return addr;
} 
//***********************************************
void stafety_shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
	if(shmctl(shmid, cmd, buf) == -1)
		error("shmctl", SHM_ERROR);
}
//***********************************************
void stafety_shmdt(const void *shmaddr)
{
	if(shmdt(shmaddr) == -1)
		error("shmdt", SHM_ERROR);
}  
//***********************************************
void debug_welcome()
{
#ifdef DEBUG_SEND_LOG
	printf("[SERV]|SEND| Welcome string\n");
#endif
}
//***********************************************
void debug_readline(const char *readline)
{
#ifdef DEBUG_RECV_LOG
	printf("[SERV]|RECV| len: %zu string: '%s'\n", strlen(readline), readline);
#endif
}
//***********************************************
void debug_client_connect(int client_id)
{
#ifdef DEBUG_RAS
	printf("[SERV] Client no.%i connected\n", client_id);
#endif
}       
//***********************************************
void debug_client_terminate(int client_id)
{
#ifdef DEBUG_RAS
	printf("[SERV] Client no.%i terminated\n", client_id);
#endif
}       
//***********************************************
void debug_fork_information(int parent_pid, int child_pid)
{
#ifdef DEBUG_FORK
	printf("[SERV]|FORK| Parent %5d fork child %5d\n", parent_pid, child_pid);
#endif
}	
//***********************************************
void debug_sig_chld_server()
{
#ifdef DEBUG_SIGNAL
	printf("[SERV]|SIGN| Signal SIGCHLD handler->sig_chld_server\n");
#endif
}
//***********************************************
void debug_sig_chld_client()
{
#ifdef DEBUG_SIGNAL
	printf("[SERV]|SIGN| Signal SIGCHLD handler->sig_chld_client\n");
#endif
}
//***********************************************
void debug_transfer_fd(int destination_fd, int source_fd)
{
#ifdef DEBUG_TRANSFER
	printf("[SERV]|TRAN| Transfer from source_fd %d to destination_fd %d\n", source_fd, destination_fd);
#endif
}
//***********************************************
void debug_transfer_fd(char *buffer)
{
#ifdef DEBUG_TRANSFER
	printf("[SERV]|TRAN| Transfer buffer len: %zu string: '%s'\n", strlen(buffer), buffer);
#endif
}
//***********************************************
void error(const char *error, bool handle_exit)
{
	if(strcmp(error, "argc") == 0)
		printf("ERROR ARGC\nPlease try ./server [PORT_NUMBER]\n");
	else
		printf("[ERROR] Happened in %s executing\n", error);
//*---------------------------------------------*    
	if(handle_exit)
		exit(0);
}
//***********************************************
void transfer_fd(int destination_fd, int source_fd)
{
	int len;
	char buffer[1024];
//*---------------------------------------------*    
	debug_transfer_fd(destination_fd, source_fd);
//*---------------------------------------------*    
	while(true)
	{ 
		len = read(source_fd, buffer, sizeof(buffer) - 1);
//*---------------------------------------------*    
		if(len == 0)
			break;
//*---------------------------------------------*   
		if(len > 0)
		{
			buffer[len] = '\0';
			debug_transfer_fd(buffer);
			write(destination_fd, buffer, len);
		}
	}
}
/***********************************************



void sig_chld_command(int signo)
{
    pid_t pid;
    int stat;
    
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("[SERV](SIGN) Call sig_chld_command\n");
}
-----------------------------
found 
//***********************************************/
