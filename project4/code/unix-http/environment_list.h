#ifndef __ENVIRONMENT_LIST__
#define __ENVIRONMENT_LIST__
//***********************************************
#include "safety_function.h"
//***********************************************
#define DEBUG_ENVIRONMENT_LIST
//************************************************
class ENVIRONMENT_LIST
{
    public:
        ENVIRONMENT_LIST();
        ~ENVIRONMENT_LIST();

        void set_environment(const char *name, const char *value);

        char* get_environment(const char *name);
		char* print_environment(const char *name);

        char** get_name();
        char** get_value();
        char** get_vector();
        char** get_header();
        
        void debug_show();
    private:
        int environment_sum;
        char **environment[4];
};
//***********************************************
#endif
