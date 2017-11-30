## 2013 NCTU INP

### Outline

This repository contains 4 projects:

### Project 1 spec

1. There is no limitation of length of single-line input.
  There may be huge number of commands in a single-line input.

2. There must be one or more spaces between commands and symbols (or arguments.), but no spaces between pipe and numbers.

	eg. cat hello.txt | number
		cat hello.txt |4

3. There will not be any '/' character in demo input.
4. Pipe ("|") will not come with "printenv" and "setenv."

### Project 2 spec

In this homework, you are asked to design chat-like systems, called remote working systems (server only). In this system, users can meet with/work with/talk to/make friends. Basically, this system supports all functions, as you did in the first project. In addition, all clients can see what all on-line users are working.

### Project 3 spec

#### Part1 CGI
1. You must implement a CGI program to provide users to do batch process in your ras/rwg server.
2. It must work correctly with the http server used in your development environment(e.g. workstations of dept. CS).
3. The number of target servers is not greater than 5.
4. The information about batch is sent by users by GET method.
5. Those parameters must be in two form. One is that hostname, port, and file name are provided, and the other is that hostname is not specified.
6. Your CGI program should show one table which lists the results from your ras/rwg servers.
7. There are five cells in the table, and the results from your ras/rwg server must be listed in corresponding cell.
   That is, the content from 1st ras/rwg server will not be displayed in 2nd cell to 5th cell.
8. You should not use fork() in CGI, instead, you should use non-blocking connect/select/read/write.
9. A large file will be tested, that is, you should not suppose that write will always success. You should handle write event properly.

#### Part2 http server in UNIX

1. You must implement a http server which can invoke requested CGI program to execute.
2. Only parameters in GET method should be processed. 
3. All files name of which ends with ".cgi" should be interpreted as a executable file and should be invoked.
4. It is not cared if requested file does not exist.
5. You have to setup all the environmant variables (totally 9) in page 8 of http.pdf when you execute CGI program, and the content of these environmant variables except "QUERY_STRING" can be everything you want.(Content of QUERY_STRING MUST be correct.)
    Ex: REMOTE_ADDR="140.113.185.117" or "remote address" or "everything I want"
6. Don't use the http server provided by CS workstation.

#### Part3 http server + CGI in Windows

1. You are asked to implement a simple http server which only provides the remote batch service, that is, it is the combination of 1st part and 2nd part.
2. This server must be implemented by winsock.
3. The client connections(i.e. the connections to your ras/rwg server) must be implemented in event driven model(WSAAsyncSelect).
4. The functionality of the CGI part must be same with 1.3 ~ 1.7 and 1.9.

### Project 4 spec

In this project, you are asked to implement the SOCKS 4 firewall protocol in the application layer of the OSI model.

SOCKS is similar to a proxy (i.e. intermediary-program) that acts as both server and client for the purpose of making request on behalf of other clients. Because the SOCKS protocol is independent of application protocols, it can be used for many different services: telnet, ftp, www, etc.

There are two types of the SOCKS operations (i.e. CONNECT and BIND). You have to implement both of them.
