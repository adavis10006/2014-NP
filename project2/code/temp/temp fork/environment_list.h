#ifndef __ENVIRONMENT_LIST__
#define __ENVIRONMENT_LIST__
//***********************************************
#include "socket_header.h"
//************************************************
class ENVIRONMENT_LIST
{
public:
	ENVIRONMENT_LIST();
	~ENVIRONMENT_LIST();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	void set_environment(const char *name, const char *value);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	char* print_environment(const char *name);
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	char** get_vector();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
	void debug_show();
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*	
private:
	int environment_sum;
	char **environment[3];
};
//***********************************************
#endif
