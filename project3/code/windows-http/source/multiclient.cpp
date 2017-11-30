#include "multiclient.h"
//***********************************************
/* 
	RECHECK 
		all recv send read write error
		nonblocking buffer
		new delete

		do the same thing to CGI and http
*/	
//***********************************************
const char *header = 
"Content-type: text/html; charset=utf-8\r\n"
"Cache-Control: max-age=0\r\n"
"\r\n";
const char *template_body_head =
"<html>\n"
"<head>\n"
"\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n"
"\t<title>Network Programming Homework 3</title>\n"
"</head>\n"
"<body bgcolor=#336699>\n"
"\t<font face=\"Courier New\" size=2 color=#FFFF99>\n";
const char *template_table_head =
"\t<table width=\"800\" border=\"1\">\n"
"\t\t<tr>\n"
"\t\t<!--td>140.113.210.145</td><td>140.113.210.145</td><td>140.113.210.145</td><td>140.113.210.145</td><td>140.113.210.145</td></tr-->\n";
const char *template_table_body =
"\t\t<tr>\n"
"\t\t<td valign=\"top\"><pre id=\"m0\"></pre></td><td valign=\"top\"><pre id=\"m1\"></pre></td><td valign=\"top\"><pre id=\"m2\"></pre></td><td valign=\"top\"><pre id=\"m3\"></pre></td><td valign=\"top\"><pre id=\"m4\"></pre></td></tr>\n"
"\t</table>\n";
const char *template_body_end =
"\t</font>\n"
"</body>\n"
"</html>\n";
//***********************************************
void get_parameter(char **query_from, ENVIRONMENT_LIST *parameter)
{
    char *seperate_pointer = strchr(*query_from, '='), *end_pointer = strchr(*query_from, '&');

    if(seperate_pointer == NULL || (end_pointer != NULL && end_pointer - seperate_pointer < 0 ))
    {
        *query_from = NULL;

        error_handle("parameter: parameter is not legal\n");
		return;
    }

    if(end_pointer == NULL)
        end_pointer = *query_from + strlen(*query_from);

    char name[BUFFER + 1], value[BUFFER + 1];
	
	memset(name, 0, sizeof(name));
	memset(value, 0, sizeof(value));

    strncpy(name, *query_from, seperate_pointer - *query_from);
    strncpy(value, seperate_pointer + 1, end_pointer - (seperate_pointer + 1));
	
	parameter->set_environment(name, value);

    *query_from = end_pointer == *query_from + strlen(*query_from) ? NULL : end_pointer + 1;
}
//***********************************************
MULTICLIENT::MULTICLIENT(int socket_fd, const char *query_string, const char *temp_exec_path): main_socket_fd(socket_fd)
{
    exec_path = new char[strlen(temp_exec_path)];
	strcpy(exec_path, temp_exec_path);
	debug_handle("exec_path: %s\n", exec_path);
	
	web_output(socket_fd, "%s", header);

    if(!parse_query(query_string))
		return;

    multiclient();
}
//***********************************************
bool MULTICLIENT::parse_query(const char *query_string)
{
    char *host = NULL, *port = NULL, *batch = NULL, *query_from = (char*) query_string;

    while(query_from != NULL)
        get_parameter(&query_from, &parameter);

    parameter.debug_show();
    
    for(int i = 0; i < 5; i++)
    {
        char host_buffer[BUFFER + 1], port_buffer[BUFFER + 1], batch_buffer[BUFFER + 1];
        
		memset(host_buffer, 0, sizeof(host_buffer));
		memset(port_buffer, 0, sizeof(port_buffer));
		memset(batch_buffer, 0, sizeof(host_buffer));

        sprintf(host_buffer, "h%d", i + 1);
        sprintf(port_buffer, "p%d", i + 1);
        sprintf(batch_buffer, "f%d", i + 1);

        host = parameter.get_environment(host_buffer);
        port = parameter.get_environment(port_buffer);
        batch = parameter.get_environment(batch_buffer);

        if(host == NULL || port == NULL || batch == NULL)
		{
            error_handle("parse_query no result\n");
			
			web_error(main_socket_fd, "parse_query no result\n");

			return false;
		}

        client_info.push_back(new CLIENT(host, atoi(port), batch));
    }

    debug_client_info();

	return true;
}
//***********************************************
void MULTICLIENT::multiclient()
{
    web_output(main_socket_fd, "%s", template_body_head);
    web_output(main_socket_fd, "%s", template_table_head);

    web_output(main_socket_fd, "\t\t");
    for(int i = 0; i < 5; i++)
    {
        char buffer[BUFFER + 1];

		memset(buffer, 0, sizeof(buffer));
		
        if(client_info[i]->client_state == CLIENT_CLOSE)
            sprintf(buffer, "<td>%s</td>", client_info[i]->host);
        else
            sprintf(buffer, "<td>no client</td>");

        web_output(main_socket_fd, "%s", buffer);
    }
    web_output(main_socket_fd, "\n");   

    web_output(main_socket_fd, "%s", template_table_body);

    connection();

    web_output(main_socket_fd, "%s", template_body_end);
}
//***********************************************
void MULTICLIENT::connection()
{
    int max_fd = -1;
    fd_set read_fd, write_fd, except_fd;

    for(int i = 0; i < client_info.size(); i++)
        if(client_info[i]->client_state == CLIENT_CLOSE)
        {
            struct hostent *host_info = gethostbyname(client_info[i]->host);
            if( host_info == NULL)
            {
                error_handle("gethostbyname: host name cannot be resolved\n");
                client_info[i]->client_state = CLIENT_NOT_EXIST;
                continue;
            }

            client_info[i]->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            memset(&(client_info[i]->client_sockaddr_in), 0, sizeof(client_info[i]->client_sockaddr_in));
            client_info[i]->client_sockaddr_in.sin_family = AF_INET;
            client_info[i]->client_sockaddr_in.sin_port = htons(client_info[i]->port);
            memcpy( &(client_info[i]->client_sockaddr_in.sin_addr), host_info->h_addr, sizeof(host_info->h_addr));

#ifdef _WIN32
			u_long sockNonBlock = 1; 
			if(ioctlsocket(client_info[i]->socket_fd, FIONBIO, &sockNonBlock) != 0)
			{
				error_handle("Fail to set the socket to non-blocking mode\n");
				
				//web_error(client_info[i]->socket_fd, "Fail to set the socket to non-blocking mode\n");
				web_output(main_socket_fd, "Fail to set the socket to non-blocking mode\n");
				
				return;
			}
#else
            int flag = fcntl(client_info[i]->socket_fd, F_GETFL, 0);
            fcntl(client_info[i]->socket_fd, F_SETFL, flag | O_NONBLOCK);
#endif
			char exec_buffer[BUFFER + 1];

			memset(exec_buffer, 0, sizeof(exec_buffer));
			
			sprintf(exec_buffer, "%s%s", exec_path, client_info[i]->batch);
			debug_handle("batch_file path: %s\n", exec_buffer);

            if((client_info[i]->file_pointer = fopen(exec_buffer, "r")) == NULL)
            {
                error_handle("batch file not exist\n", false);
                client_info[i]->client_state = CLIENT_NOT_EXIST;
                continue;
            }

            if(connect(client_info[i]->socket_fd, (struct sockaddr*) &(client_info[i]->client_sockaddr_in), sizeof(client_info[i]->client_sockaddr_in)) <= 0 )
            {
#ifdef _WIN32
				if(WSAGetLastError() != WSAEWOULDBLOCK)
#else
				if(errno != EINPROGRESS)
#endif
				{
                    error_handle("connect: cannot connect to client\n", false);
                    client_info[i]->client_state = CLIENT_NOT_EXIST;
                    continue;
                }

                client_info[i]->client_state = CLIENT_INITIAL;
            }   
            else
            {
                error_handle("connect: finish connect before do select\n", false);

                client_info[i]->client_state = CLIENT_WAIT_READ;
            }

            max_fd = client_info[i]->socket_fd > max_fd ? client_info[i]->socket_fd : max_fd; 
        }

    while(true)
    {
        bool all_client_finish = true;

        FD_ZERO(&read_fd);
        FD_ZERO(&write_fd);
		FD_ZERO(&except_fd);

        for(int i = 0; i < client_info.size(); i++)
            if(client_info[i]->client_state == CLIENT_INITIAL)
			{     
				FD_SET(client_info[i]->socket_fd, &read_fd);
				all_client_finish = false;
			}
            else if(client_info[i]->client_state == CLIENT_WAIT_READ)
            {
				FD_SET(client_info[i]->socket_fd, &read_fd);
				all_client_finish = false;
			}
			else if(client_info[i]->client_state == CLIENT_WAIT_WRITE && client_info[i]->write_input <= client_info[i]->write_output)
            {    
#ifdef _WIN32
				FD_SET(client_info[i]->socket_fd, &except_fd);
#endif
				FD_SET(client_info[i]->socket_fd, &write_fd);
				all_client_finish = false;
			}

        if(all_client_finish)
            break;

        int ready_client_amount;

		if(ready_client_amount= select(max_fd + 1, &read_fd, &write_fd, NULL, NULL) , 0)
		{
#ifdef _WIN32
			if(WSAGetLastError() != WSAEWOULDBLOCK)
#else
			if(errno != EINPROGRESS)
#endif
				continue;
		}

        for(int i = 0; i < client_info.size(); i++)
        {
            if(client_info[i]->client_state == CLIENT_NOT_EXIST)
                continue;
            if(client_info[i]->client_state == CLIENT_CLOSE)
            {
                error_handle("client: client not handle initial");

                continue;
            }

            int socket_fd = client_info[i]->socket_fd;

            if(FD_ISSET(socket_fd, &read_fd) && client_info[i]->client_state == CLIENT_INITIAL)
            {
                int error_type;
				socklen_t error_len = sizeof(error_type);

                if(getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (char*) &error_type, &error_len) < 0 || error_type != 0)
                {
                    error_handle("connect: connection has error happened\n");

                    FD_CLR(socket_fd, &read_fd);

					return;
                }
                else
                {
                    client_info[i]->client_state = CLIENT_WAIT_READ;

                    if(--ready_client_amount <= 0)
                        break;
                }
            }

            if(FD_ISSET(socket_fd, &read_fd) && client_info[i]->client_state == CLIENT_WAIT_READ)
            {
                int read_len;
				char read_buffer[SOCKET_BUFFER + 1];
				memset(read_buffer, 0 ,sizeof(read_buffer));

				if((read_len = recv(socket_fd, read_buffer, SOCKET_BUFFER, 0)) < 0)
                {    
                    if(WSAGetLastError() != WSAEINTR)
                    	error_handle("read: read error happened\n");
				}
                else if(read_len == 0)
                {
                    closesocket(socket_fd);
                    client_info[i]->client_state = CLIENT_NOT_EXIST;
                }
                else
                {
                    web_script(main_socket_fd, i, read_buffer, read_len);
					
					if(read_len == 2 && strncmp(read_buffer, "% ", 2) == 0)
                        client_info[i]->client_state = CLIENT_WAIT_WRITE;
                }

                if(--ready_client_amount <= 0)
                    break;
            }

            if(FD_ISSET(socket_fd, &write_fd) && client_info[i]->client_state == CLIENT_WAIT_WRITE)
            {
                int read_len, write_len;

                int read_limit = &(client_info[i]->write_buffer[SOCKET_BUFFER]) - client_info[i]->write_output;

                if(client_info[i]->write_input == client_info[i]->write_output)
				{
					char read_buffer[SOCKET_BUFFER + 1];
					memset(read_buffer, 0, sizeof(read_buffer));

					fgets(read_buffer, read_limit, client_info[i]->file_pointer); 

					read_len = strlen(read_buffer);
					strcpy(client_info[i]->write_output, read_buffer);
				}

                web_script(main_socket_fd, i, client_info[i]->write_output, read_len);

                client_info[i]->write_output += read_len;

                int write_limit = client_info[i]->write_output - client_info[i]->write_input;

                if((write_len = send(socket_fd, client_info[i]->write_input, write_limit, 0)) < 0) 
                {
                    if(WSAGetLastError() != WSAEINTR)
					    error_handle("write: write error happened\n");
                }
                else
                {
                    if(write_len == write_limit)
                    {    
                        client_info[i]->write_input = client_info[i]->write_output = client_info[i]->write_buffer;
                        client_info[i]->client_state = CLIENT_WAIT_READ;
                    }
                    else
                        client_info[i]->write_input += write_len;
                }

                if(--ready_client_amount <= 0)
                    break;
            }
        }  
    }
}
//***********************************************
void MULTICLIENT::debug_client_info()
{
#if defined(DEBUG_CLIENT)
    fprintf(stderr, "client_info length: %lu\n", client_info.size());
    for(int i = 0; i < client_info.size(); i++)
    {
        fprintf(stderr, "number %d:\n", i);
        fprintf(stderr, "\thost: '%s'\n", client_info[i]->host);
        fprintf(stderr, "\tport: '%d'\n", client_info[i]->port);
        fprintf(stderr, "\tbatch: '%s'\n", client_info[i]->batch);
    }  
#endif   
}
//***********************************************
