#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__
//************************************************
#include <arpa/inet.h>
#include <deque>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <string>
//************************************************
#define SERVER_VERSION "2.0.0"
#define RAS_FILE "RAS"
//************************************************
#define SA struct sockaddr
#define SERVER_PORT 5566
#define MAX_CLIENTS 50
#define READ_BUFFER 1024
#define BUFFER 1024
#define SINGLE_COMMAND_MAXBUFFER 1024
//************************************************
#define FILE_MODE 0777
//************************************************
#define SHM_ERROR false
//************************************************
#define EXEC_UNKNOWN 255
//************************************************
//#define DEBUG_SELECT
//#define DEBUG_RAS
//#define DEBUG_CLIENT_LIST
//#define DEBUG_COMMAND
//#define DEBUG_COMMAND_PARSE_ARGV
//#define DEBUG_FIFO
//#define DEBUG_FIFO_REDIRECT
//#define DEBUG_COMMAND_EXEC_ERROR
//#define DEBUG_MESSAGE
/************************************************
#define DEBUG_RAS
#define DEBUG_SINGAL
#define DEBUG_FORK
#define DEBUG_SELECT
#define DEBUG_CLIENT_LIST
#define DEBUG_ENVIRONMENT_LIST
#define DEBUG_PIPE
#define DEBUG_PIPE_LIST
#define DEBUG_FIFO
#define DEBUG_COMMAND
#define DEBUG_PIPE_VECTOR
#define DEBUG_COMMAND_LIST
#define DEBUG_TRANSFER
#define DEBUG_MESSAGE
//************************************************
#define DEBUG_SEND_LOG
#define DEBUG_RECV_LOG
#define DEBUG_PIPE_BUILDIN
#define DEBUG_PIPE_EXIST
#define DEBUG_PIPE_SET
#define DEBUG_PIPE_REDIRECT
#define DEBUG_PIPE_CLOSE
#define DEBUG_PIPE_LIST_BUILDIN
#define DEBUG_PIPE_LIST_DELETE
#define DEBUG_PIPE_LIST_CLEAR
#define DEBUG_COMMAND_BUILDIN
#define DEBUG_COMMAND_CREATE
#define DEBUG_COMMAND_PARSE
#define DEBUG_COMMAND_PARSE_ARGV
#define DEBUG_COMMAND_EXEC_ERROR
#define DEBUG_PIPE_VECTOR
#define DEBUG_PIPE_VECTOR_EXIST
#define DEBUG_PIPE_VECTOR_DELETE
#define DEBUG_PIPE_VECTOR_CLEAR
#define DEBUG_COMMAND_LIST_BUILDIN
#define DEBUG_COMMAND_LIST_DELETE
#define DEBUG_COMMAND_LIST_CLEAR
#define DEBUG_FIFO_REDIRECT

#define ERROR_PRINTENV
#define ERROR_SETENV
//************************************************/
enum READLINE_RETURN
{
	READLINE_UNDONE,
	READLINE_FINISH,
	READLINE_EOF,
	READLINE_ERROR
};
//************************************************
enum COMMAND_RETURN
{
	COMMAND_EXIT,
	COMMAND_ERROR,
	COMMAND_SUCCESS
};
//************************************************
enum PIPE_INOUT
{
	PIPE_READ = 0,
	PIPE_WRITE = 1
};
//************************************************
enum PIPE_TYPE
{
	PIPE_SINGLE,
	PIPE_FRONT,
	PIPE_MIDDLE,
	PIPE_END
};
//************************************************
enum PARSE_RESULT
{
	PARSE_SUCCESS,
	PARSE_ERROR
};
//************************************************
enum EXEC_RESULT
{
	EXEC_SUCCESS,
	EXEC_FORBID,
	EXEC_ERROR
};
//************************************************
#endif
