#ifndef __MULTICLIENT__
#define __MULTICLIENT__
//***********************************************
#define DEBUG_CLIENT
//***********************************************
#include "environment_list.h"
#include <deque>
//***********************************************
struct CLIENT 
{
    char *host, *batch;
    int port;
    bool exist;
    
    int socket_fd;
    struct sockaddr_in client_sockaddr_in;

    FILE *file_pointer; 
    int file_fd;
   
    CLIENT_STATE client_state;
    
    char write_buffer[SOCKET_BUFFER + 1];
    char *write_input, *write_output;
    
    CLIENT(const char *input_host,const int input_port, const char *input_batch): 
        port(input_port)
    {
        host = new char[strlen(input_host) + 1];
        batch = new char[strlen(input_batch) + 1];

        memset(host, 0, sizeof(host));
        memset(batch, 0, sizeof(batch));

        strcpy(host, input_host);
        strcpy(batch, input_batch);

        if(strlen(input_host) != 0)
            client_state = CLIENT_CLOSE;
        else 
            client_state = CLIENT_NOT_EXIST;
                   
        write_input = write_output = write_buffer;
    }
};
//***********************************************
class MULTICLIENT
{
public:
    MULTICLIENT(const char *query_string);
    
    void parse_query(const char *query_string);
    
    void multiclient();
    void connection();

    void debug_client_info();
private:
    ENVIRONMENT_LIST parameter;

    std::deque<CLIENT*> client_info;    
};
//***********************************************
#endif
