#ifndef __SERVER__
#define __SERVER__
//***********************************************
#include "socket_header.h"
#include "client_list.h"
#include "safety_function.h"
#include "command_list.h"
//***********************************************
int main(int argc, char** argv);
//***********************************************
void debug_select(CLIENT *client);
//***********************************************
#endif


