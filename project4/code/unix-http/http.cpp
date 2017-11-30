#include "http_handler.h"
//***********************************************
void HTTP(int socket_fd)
{
    HTTP_HANDLER http_handler(socket_fd);

    if(!http_handler.request())
		return;
		
    http_handler.analysis();
    http_handler.response();
}
//***********************************************
void startHTTP(int server_port)
{
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    int socket_fd, connect_fd;
    pid_t child_pid;

    fprintf(stderr, "[SERV] http server version %s starting\n", HTTP_VERSION);

    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		error_handle("socket: socket create error\n", true);
	
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

	if(bind(socket_fd, (SA*) &server_addr, sizeof(server_addr)) == -1)
        error_handle("bind: bind create error\n", true);

	if(listen(socket_fd, MAX_CLIENTS) == -1)
        error_handle("listen: listen create error\n", true);
	
	if(signal(SIGCHLD, sig_chld_server) == SIG_ERR)
        error_handle("signal: signal set error\n", true);
    
    while(true)
    {
        client_len = sizeof(client_addr);
        connect_fd = accept(socket_fd, (SA*) &client_addr, &client_len);

        if(connect_fd == -1)
            error_handle("accept: accept failed\n");

        if((child_pid = fork()) == 0)
        {
            close(socket_fd);

            HTTP(connect_fd);

            close(connect_fd);

            return;
        }
        else if(child_pid > 0)
            close(connect_fd);
        else
            error_handle("fork: fork create failed\n");
    }

    return;
}
//***********************************************
int main(int argc, char **argv)
{
    int server_port;

    if(argc == 1)
    {
        server_port = 8080;

        error_handle("no paramter input, set port as 8080\n");
    }
    else if(argc == 2)
    {
        server_port = atoi(argv[1]);

        if(server_port <= 1024 && server_port >= 65536)
            error_handle("port is invalid\n", true);
    }
    else
        error_handle("too many paramter\n", true);

    initial_http_handler();

    startHTTP(server_port);

    return 0;
}
//***********************************************
