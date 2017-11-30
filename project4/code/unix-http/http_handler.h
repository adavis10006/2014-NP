#ifndef __HTTP_HANDLER__
#define __HTTP_HANDLER__
//***********************************************
#include "environment_list.h"
#include <map>
//***********************************************
typedef char URL[BUFFER + 1];
typedef char HTTP_TYPE[BUFFER + 1];
typedef char PATH[BUFFER + 1];
typedef char FILE_TYPE[BUFFER + 1];
//***********************************************
enum REQUEST_TYPE
{
    HTTP_GET,
    HTTP_POST,
    HTTP_OTHER
};
//***********************************************
enum RESPONSE_TYPE
{
    RESPONSE_OK = 200,
    RESPONSE_FORBIDDEN = 403,
    RESPONSE_NOT_FOUND = 404
};
//***********************************************
struct HTTP_REQUEST
{
    REQUEST_TYPE request_type;
    URL url; 
    HTTP_TYPE http_type;
    ENVIRONMENT_LIST header;
};
//***********************************************
struct HTTP_ANALYSIS
{
    PATH request_file;
    FILE_TYPE file_type;
    ENVIRONMENT_LIST header;
};
//***********************************************
struct HTTP_RESPONSE
{
    HTTP_TYPE http_type;
    RESPONSE_TYPE response_type;
    ENVIRONMENT_LIST header;
};
//***********************************************
void initial_http_handler();
//***********************************************
class HTTP_HANDLER
{
    public:
        HTTP_HANDLER(int socket_fd);

        bool request();

        void analysis();

        void response();

    private:
        int socket_fd;
        
        HTTP_REQUEST http_request;
        HTTP_ANALYSIS http_analysis;
        HTTP_RESPONSE http_response; 
};
//***********************************************
#endif
