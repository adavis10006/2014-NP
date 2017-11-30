#include "environment_list.h"
//***********************************************
ENVIRONMENT_LIST::ENVIRONMENT_LIST(): environment_sum(0)
{
	for(int i = 0; i < 3; i++)
		environment[ i ] = NULL;
//*---------------------------------------------*
	set_environment("PATH", "bin:.");
}
//***********************************************
ENVIRONMENT_LIST::~ENVIRONMENT_LIST()
{
	for(int i = 0; i < 3; i++)
		for(int j = environment_sum; j >= 0; j--)
			delete(environment[i][j]);

	for(int i = 0; i < 3; i++)
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
			return;
		}
//*---------------------------------------------*
	environment_sum++;
//*---------------------------------------------*
	for(int i = 0; i < 3; i++)
	{	
		environment[i] = (char**) realloc(environment[i], (environment_sum + 1) * sizeof(char*));
		environment[i][environment_sum - 1] = new char[BUFFER];
		environment[i][environment_sum] = NULL;
	}
//*---------------------------------------------*
	strcpy(environment[0][environment_sum - 1], name);
	strcpy(environment[1][environment_sum - 1], value);
	sprintf(environment[2][environment_sum - 1], "%s=%s", environment[0][environment_sum - 1], environment[1][environment_sum - 1]);
//*---------------------------------------------*
	debug_show();
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
char** ENVIRONMENT_LIST::get_vector()
{
	return environment[2];
}
//***********************************************
void ENVIRONMENT_LIST::debug_show()
{
#if defined(DEBUG_ENVIRONMENT_LIST)
	printf("[SERV]|ENVR| Show the environment in list\n");
	printf("name\tvalue\tanser\n");
	for(int j = 0; j < environment_sum; j++, printf("\n"))
		for(int i = 0; i < 3; i++, printf("\t"))
			printf("%s", environment[i][j]);
#endif
}
//***********************************************
