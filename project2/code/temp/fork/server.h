#ifndef __SERVER__
#define __SERVER__
//***********************************************
#include "socket_header.h"
#include "client_list.h"
#include "safety_function.h"
#include "command_list.h"
#include "semaphore.h"
//***********************************************
void RAS(int socket_fd, struct sockaddr_in client_addr);
//***********************************************
int main(int argc, char** argv);
//***********************************************
#endif


