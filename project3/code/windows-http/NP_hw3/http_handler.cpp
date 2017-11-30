#include "http_handler.h"
#include "multiclient.h"
//***********************************************
namespace file_env
{
	std::map<int , std::string*> status_name;
	ENVIRONMENT_LIST file_type;
}
//***********************************************
const char *error_output_head =
"<html>\r\n"
"\t<title>ERROR</title>\r\n"
"\t<body>";
const char *error_output_end =
"</body>\r\n"
"</html>\r\n";
//***********************************************
void initial_http_handler()
{
    file_env::status_name[200] = new std::string("OK");
    file_env::status_name[403] = new std::string("FORBIDDEN");
    file_env::status_name[404] = new std::string("NOT FOUND");

	file_env::file_type.set_environment("txt", "text/plain");
    file_env::file_type.set_environment("htm", "text/html");
    file_env::file_type.set_environment("html", "text/html");
    file_env::file_type.set_environment("cgi", "text/html");
}
//***********************************************
void server_error_output(int socket_fd, const char *error_message, ...)
{
    char socket_buffer[SOCKET_BUFFER + 1];
    va_list args;

	memset(socket_buffer, 0, sizeof(socket_buffer));

    va_start(args, error_message);
    vsprintf(socket_buffer, error_message, args);
    va_end(args);

    debug_handle("%s%s%s", error_output_head, socket_buffer, error_output_end);
    server_handle(socket_fd, "%s%s%s", error_output_head, socket_buffer, error_output_end);
}
//***********************************************
void parse_HTTP_REQUEST(char *http_request_string, HTTP_REQUEST *http_request)
{
    char request_type[BUFFER + 1];
	
	memset(request_type, 0, sizeof(request_type));

    sscanf(http_request_string, "%s %s %s", request_type, http_request->url, http_request->http_type);

    if(strcmp(request_type, "GET") == 0)
        http_request->request_type = HTTP_GET;
    else if(strcmp(request_type, "POST") == 0)
        http_request->request_type = HTTP_POST;
    else
        http_request->request_type = HTTP_OTHER;
}
//***********************************************
bool parse_HTTP_HEADER(char *http_header_string, HTTP_REQUEST *http_request)
{
    char header_name[BUFFER + 1], header_value[BUFFER + 1];
	
	memset(header_name, 0, sizeof(header_name ));
	memset(header_value, 0, sizeof(header_value));

    char *separate_position = strchr(http_header_string, ':');

    if(separate_position == NULL)
    {
        error_handle("parse header: error format\n");

		return false;
    }

    strncpy(header_name, http_header_string, separate_position - http_header_string);
    strcpy(header_value, separate_position + 2);

    http_request->header.set_environment(header_name, header_value);

	return true;
}
//***********************************************
void parse_URL(PATH url, HTTP_ANALYSIS *http_analysis)
{
    char *query_position = strchr(url, '?');

    strcpy(http_analysis->request_file, $(HOME));
    strcat(http_analysis->request_file, HTTP_ROOT);

    if(query_position != NULL)
    {
        strncat(http_analysis->request_file, url, query_position - url);

        http_analysis->header.set_environment("QUERY_STRING", query_position + 1);
    }   
    else
    {
        strcat(http_analysis->request_file, url);

        http_analysis->header.set_environment("QUERY_STRING", "");
    }

    http_analysis->header.set_environment("REQUEST_METHOD", "GET");
    http_analysis->header.set_environment("SCRIPT_NAME", http_analysis->request_file);
    http_analysis->header.set_environment("REMOTE_HOST", "localhost");
    http_analysis->header.set_environment("REMOTE_ADDR", "127.0.0.1");
    http_analysis->header.set_environment("AUTH_TYPE", "");
    http_analysis->header.set_environment("REMOTE_USER", "");
    http_analysis->header.set_environment("REMOTE_IDENT", "");
    http_analysis->header.set_environment("CONTENT_LENGTH", "");

    char *path_end_position = http_analysis->request_file + strlen(http_analysis->request_file);

	char **file_type_name = file_env::file_type.get_name();

	while(file_type_name != NULL && *file_type_name != NULL)
	{   
		int extension_len = strlen(*file_type_name);

        if(strlen(http_analysis->request_file) >= extension_len && strncmp(path_end_position - extension_len, *file_type_name, extension_len) == 0)
            strcpy(http_analysis->file_type, *file_type_name);   

		file_type_name++;
	}

    if(strlen(http_analysis->file_type) == 0)
        strcpy(http_analysis->file_type, "txt"); 

    debug_handle("file_type: %s\n", http_analysis->file_type);
}
//***********************************************
/*
 * HTTP FORMAT
 * <HTTP STATUS(request or response)>
 * <HEADER...>
 * ...
 * <BLANL LINE>
 * <DATA>
 */
//***************************************
HTTP_HANDLER::HTTP_HANDLER(int socket_fd): socket_fd(socket_fd) 
{
	memset(http_request.url, 0, sizeof(http_request.url));
	memset(http_request.http_type, 0, sizeof(http_request.http_type));
	memset(http_analysis.request_file, 0, sizeof(http_analysis.request_file));
	memset(http_analysis.file_type, 0, sizeof(http_analysis.file_type));
	memset(http_response.http_type, 0, sizeof(http_response.http_type));
}
//***********************************************
bool HTTP_HANDLER::request()
{
    char socket_buffer[SOCKET_BUFFER + 1];
	int read_len;

	memset(socket_buffer, 0, sizeof(socket_buffer));
	read_len = safety_readline(socket_fd, socket_buffer, SOCKET_BUFFER);
    
	debug_handle("%s", socket_buffer);
    // request
    parse_HTTP_REQUEST(socket_buffer, &http_request);
    // header
    while(true)
    {
        memset(socket_buffer, 0, sizeof(socket_buffer));
		read_len = safety_readline(socket_fd, socket_buffer, SOCKET_BUFFER);
        
		debug_handle("%s", socket_buffer);

        if(strcmp(socket_buffer, "\n") == 0 || strcmp(socket_buffer, "\r\n") == 0)
            break;

        if(!parse_HTTP_HEADER(socket_buffer, &http_request))
			return false;
    }

	return true;
    //while((read_len = safety_readline(socket_fd, socket_buffer, SOCKET_BUFFER)) > 0);
}
//***********************************************
void HTTP_HANDLER::analysis()
{
    if(http_request.request_type != HTTP_GET)
    {    
        http_response.response_type = RESPONSE_FORBIDDEN;

        return;
    }

    parse_URL(http_request.url, &http_analysis);

    debug_handle("path: %s\n\n", http_analysis.request_file);

    FILE *request_file_ptr; 

    if((request_file_ptr = fopen(http_analysis.request_file, "r")) == NULL)
        http_response.response_type = RESPONSE_NOT_FOUND;
    else
    {
        http_response.response_type = RESPONSE_OK;

        fclose(request_file_ptr);
    }
}
//***********************************************
void HTTP_HANDLER::response()
{
    strcpy(http_response.http_type, http_request.http_type);

    debug_handle("%s %d %s\n", http_response.http_type, http_response.response_type, file_env::status_name[http_response.response_type]->c_str());
    server_handle(socket_fd, "%s %d %s\n", http_response.http_type, http_response.response_type, file_env::status_name[http_response.response_type]->c_str());

    char **header_list = http_response.header.get_header();

    while(header_list != NULL && *header_list != NULL)
    {
        debug_handle("%s", *header_list);
        server_handle(socket_fd, "%s", *header_list);

        header_list++;
    }

    if(http_response.response_type != RESPONSE_OK)
    {
		//debug_handle("Content-Type: text/html\r\n\r\n");
        server_handle(socket_fd, "Content-Type: text/html\r\n\r\n");

        server_error_output(socket_fd, "RESPONSE is %d %s\0", http_response.response_type, file_env::status_name[http_response.response_type]->c_str());

        return;
    }
    else
        if(strcmp(http_analysis.file_type, "cgi") == 0)
        {
            //debug_handle("handle CGI\r\n\r\n");
            http_analysis.header.debug_show();

			char buffer[BUFFER + 1];

			memset(buffer, 0, sizeof(buffer));
			strcpy(buffer, http_analysis.request_file);

			for(int i = strlen(buffer) - 1; i >= 0; i--)
                if(buffer[i] != '/')
                    buffer[i] = '\0';
				else 
					break;
              
			const char *query_string = http_analysis.header.get_environment("QUERY_STRING");
			
			if(query_string == NULL)
			{	
				error_handle("No return pointer for QUERY_STRING\n");
				
				web_error(socket_fd, "No return pointer for QUERY_STRING\n");
			}

			MULTICLIENT multiclient(socket_fd, query_string, buffer);
        }
        else
        {
            //debug_handle("Content-Type: %s\r\n\r\n", file_env::file_type.get_environment(http_analysis.file_type));
            server_handle(socket_fd, "Content-Type: %s\r\n\r\n", file_env::file_type.get_environment(http_analysis.file_type));

            FILE *file_pointer = fopen(http_analysis.request_file, "r");

            if(file_pointer == NULL)
            {
                error_handle("fopen: file is busy\n");

                server_error_output(socket_fd, "fopen open fail!");

                return;
            }
            else
            {
                char socket_buffer[SOCKET_BUFFER + 1];

				memset(socket_buffer, 0, sizeof(socket_buffer));
				while(fgets(socket_buffer, SOCKET_BUFFER, file_pointer) != NULL)
                {
                    //debug_handle("%s", socket_buffer);
                    server_handle(socket_fd, "%s", socket_buffer);
                }
            }

			//debug_handle("\r\n");
            server_handle(socket_fd, "\r\n");
        }
}
//***********************************************

