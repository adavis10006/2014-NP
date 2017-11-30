#include "multiclient.h"
//***********************************************
void startCGI()
{
    const char *query_string = getenv("QUERY_STRING");
    if(query_string == NULL)
        error_handle("No return pointer for QUERY_STRING\n", true);

    MULTICLIENT multiclient(query_string);
}
//***********************************************
int main(int argc, char **argv)
{
    startCGI();

    error_handle("CGI done.\n");

    return 0;
}
//***********************************************
