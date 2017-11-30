#include "environment_list.h"
//***********************************************
ENVIRONMENT_LIST::ENVIRONMENT_LIST(): environment_sum(0)
{
    for(int i = 0; i < 4; i++)
        environment[i] = NULL;
}
//***********************************************
ENVIRONMENT_LIST::~ENVIRONMENT_LIST()
{
    for(int i = 0; i < 4; i++)
        for(int j = environment_sum; j >= 0; j--)
			if(environment[i] != NULL && environment[i][j] != NULL)
				delete(environment[i][j]);

    for(int i = 0; i < 4; i++)
		if(environment[i] != NULL)
			free(environment[i]);
}
//***********************************************
void ENVIRONMENT_LIST::set_environment(const char *name, const char *value)
{
    for(int i = 0; i < environment_sum; i++)
        if(strcmp(environment[0][i], name) == 0)
        {
            strcpy(environment[1][i], value);
            sprintf(environment[2][i], "%s=%s", environment[0][i], environment[1][i]);
            sprintf(environment[3][i], "%s: %s\n", environment[0][i], environment[1][i]);
            return;
        }

    environment_sum++;

    for(int i = 0; i < 4; i++)
    {   
        environment[i] = (char**) realloc(environment[i], (environment_sum + 1) * sizeof(char*));
        environment[i][environment_sum - 1] = new char[BUFFER + 1];

        memset(environment[i][environment_sum - 1], 0, sizeof(environment[i][environment_sum - 1]));

        environment[i][environment_sum] = NULL;
    }

    strcpy(environment[0][environment_sum - 1], name);
    strcpy(environment[1][environment_sum - 1], value);
    sprintf(environment[2][environment_sum - 1], "%s=%s", environment[0][environment_sum - 1], environment[1][environment_sum - 1]);
    sprintf(environment[3][environment_sum - 1], "%s: %s\r\n", environment[0][environment_sum - 1], environment[1][environment_sum - 1]);
}
//***********************************************
char* ENVIRONMENT_LIST::get_environment(const char *name)
{
    for(int i = 0; i < environment_sum; i++)
        if(strcmp(environment[0][i], name) == 0)
            return environment[1][i];
    return NULL;
}
//***********************************************
char* ENVIRONMENT_LIST::print_environment(const char *name)
{
    for(int i = 0; i < environment_sum; i++)
        if(strcmp(environment[0][i], name) == 0)
            return environment[2][i];
    return NULL;
}
//***********************************************
char** ENVIRONMENT_LIST::get_name()
{
    if(environment_sum == 0)
        return NULL;

    return environment[0];
}
//***********************************************
char** ENVIRONMENT_LIST::get_value()
{
    if(environment_sum == 0)
        return NULL;

    return environment[1];
}
//***********************************************
char** ENVIRONMENT_LIST::get_vector()
{
    if(environment_sum == 0)
        return NULL;

    return environment[2];
}
//***********************************************
char** ENVIRONMENT_LIST::get_header()
{
    if(environment_sum == 0)
        return NULL;

    return environment[3];
}
//***********************************************
void ENVIRONMENT_LIST::debug_show()
{
#if defined(DEBUG_ENVIRONMENT_LIST)
    debug_handle("Show the environment in list\n");
    debug_handle("name\tvalue\n");
    for(int j = 0; j < environment_sum; j++, debug_handle("\n"))
        for(int i = 0; i < 2; i++, printf("\t"))
            debug_handle("%s\t", environment[i][j]);
#endif
}
//***********************************************
