#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__
//***********************************************
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Windowsx.h>
#include <deque>
#include <iostream>
#include <string>
#include "resource.h"
//***********************************************
#define HTTP_VERSION "0.0.0"
#define HTTP_PORT 23456
#define $(HOME) "D://Temp//"      //getenv("HOME")
#define HTTP_ROOT ""                    // "/public_html/cgi-bin/"
//***********************************************
#define BUFFER 1024
#define SOCKET_BUFFER 102400
//***********************************************
enum CLIENT_STATE
{
    CLIENT_NOT_EXIST,
    CLIENT_CLOSE,
    CLIENT_INITIAL,
    CLIENT_WAIT_READ,
    CLIENT_WAIT_WRITE
};
//***********************************************
#endif
