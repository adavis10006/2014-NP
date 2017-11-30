#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
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
#include <deque>

#define SA 			struct sockaddr
#define MAXCLIENTS  10
#define MAXLINE     1024
#define MAXBUFFER   2000000
#define SERVERPORT  12345

enum COMMAND_RETURN
{
	EXIT,
	ERROR,
	SUCCESS
};

enum PIPE_INOUT
{
	PIPE_READ = 0,	//read end
	PIPE_WRITE = 1	//write end 
};

enum PIPE_TYPE
{
	PIPE_SINGLE,
	PIPE_FRONT,
	PIPE_MIDDLE,
	PIPE_END
};

struct Pipe
{
	int pipefd[ 2 ];
	int middlefd[ 2 ];
	//int errorfd[ 2 ];
	bool create_pipe;
	bool create_middle;
	//bool create_error;

	Pipe()
	{
		create_pipe = create_middle = false;//= create_error = false;
	}

	~Pipe()
	{
		if( create_pipe )
		{
			close( pipefd[ PIPE_WRITE ] );
			close( pipefd[ PIPE_READ ] );
		}
		if( create_middle )
		{
			close( middlefd[ PIPE_WRITE ] );
			close( middlefd[ PIPE_READ ] );
		}
		/*if( create_error )
		{
			close( errorfd[ PIPE_WRITE ] );
			close( errorfd[ PIPE_READ ] );
		}*/
	}

};

struct Command
{
	char* command;

	Command( char* string, int len )
	{
	 command = new char[ len + 1 ];
		strncpy( command, string, len );
		command[ len ] = '\0';
	}

	~Command()
	{
		//printf( "Delete Command %s\n", command );
		delete( command );
	}
};

void transfer_fd( int destination_fd, int source_fd )
{
	int len;
	char buffer[ 1024 ];

	printf( "[SERV](PIPE) Transfering...\n" );

	while( true )
	{ 
		len = read( source_fd, buffer, sizeof( buffer ) - 1 );
		
		if( len == 0 )
			break;
	
		if( len > 0 )
		{
			buffer[ len ] = '\0';
			printf( "%s", buffer );
			write( destination_fd, buffer, len );
		}
	}

	printf( "[SERV](PIPE) Transfer done\n" );
}

void sig_chld_server( int signo )
{
    pid_t pid;
    int stat;
    
    while( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
		printf( "[SERV](SIGN) Call sig_chld_server stat %d\n", stat );
}

void sig_chld_command( int signo )
{
    pid_t pid;
    int stat;
    
    while( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
		printf( "[SERV](SIGN) Call sig_chld_command\n" );
}

class Pipe_list
{
public:
	Pipe_list()
	{
		;
	}

	~Pipe_list()
	{
		while( !root.empty() )
			delete_front();
	}

	void check_pipe_number( int target )
	{
		while( root.size() <= target )
		{
			root.push_back( new Pipe );
			printf( "Create pipe at %x\n", ( int* ) root[ root.size()-1 ] );
		}
	}

	void check_pipe( int pipe_number )
	{
		printf( "[SERV](PIPE) Check pipe at address %x\n", ( int* ) root[ pipe_number ] );
		
		if( !( root[ pipe_number ]->create_pipe ) )
		{
			printf( "[SERV](PIPE) Create pipe at address %x\n", ( int* ) root[ pipe_number ] );
			root[ pipe_number ]->create_pipe = true;
			pipe( root[ pipe_number ]->pipefd );
		}

		if( !( root[ pipe_number ]->create_middle ) )
		{
			printf( "[SERV](PIPE) Create middle at address %x\n", ( int* ) root[ pipe_number ] );
			root[ pipe_number ]->create_middle = true;
			pipe( root[ pipe_number ]->middlefd );
		}
/*
		if( !( root[ pipe_number ]->create_error ) )
		{
			printf( "[SERV](PIPE) Create error at address %x\n", ( int* ) root[ pipe_number ] );
			root[ pipe_number ]->create_error = true;
			pipe( root[ pipe_number ]->errorfd );
		}*/
	}

	void redirect_pipe_single( int sockfd, bool handle_error )
	{
		if( root[ 0 ]->create_pipe )
		{
			printf( "Redirect pipe to input %x\n", root[ 0 ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
			dup2( root[ 0 ]->pipefd[ PIPE_READ ], STDIN_FILENO );
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
		}
		
		dup2( sockfd, STDOUT_FILENO );
		if( handle_error )
			dup2( sockfd, STDERR_FILENO );	
	}

	void redirect_pipe_front( int pipe_number, bool handle_error )
	{
		if( root[ 0 ]->create_pipe )
		{
			printf( "Redirect pipe to input %x\n", root[ 0 ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
			dup2( root[ 0 ]->pipefd[ PIPE_READ ], STDIN_FILENO );
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
		}

		printf( "Redirect output to pipe %x\n", root[ pipe_number ] );
		close( root[ pipe_number ]->middlefd[ PIPE_READ ] );
		dup2( root[ pipe_number ]->middlefd[ PIPE_WRITE ], STDOUT_FILENO );
		if( handle_error )
			dup2( root[ pipe_number ]->middlefd[ PIPE_WRITE ], STDERR_FILENO );
		close( root[ pipe_number ]->middlefd[ PIPE_WRITE ] );
	}

	void redirect_pipe_front( Pipe_list* pipe_destination, bool handle_error )
	{
		if( root[ 0 ]->create_pipe )
		{
			printf( "Redirect pipe to input %x\n", root[ 0 ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
			dup2( root[ 0 ]->pipefd[ PIPE_READ ], STDIN_FILENO );
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
		}

		printf( "Redirect output to pipe %x\n", pipe_destination->root[ 0 ] );
		close( pipe_destination->root[ 0 ]->middlefd[ PIPE_READ ] );
		dup2( pipe_destination->root[ 0 ]->middlefd[ PIPE_WRITE ], STDOUT_FILENO );
		if( handle_error )
			dup2( pipe_destination->root[ 0 ]->middlefd[ PIPE_WRITE ], STDERR_FILENO );
		close( pipe_destination->root[ 0 ]->middlefd[ PIPE_WRITE ] );
	}

	void redirect_pipe_middle( int pipe_number, bool handle_error )
	{
		if( root[ 0 ]->create_pipe )
		{
			printf( "Redirect pipe to input %x\n", root[ 0 ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
			dup2( root[ 0 ]->pipefd[ PIPE_READ ], STDIN_FILENO );
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
		}

		printf( "Redirect output to pipe %x\n", root[ pipe_number ] );
		close( root[ pipe_number ]->middlefd[ PIPE_READ ] );
		dup2( root[ pipe_number ]->middlefd[ PIPE_WRITE ], STDOUT_FILENO );
		if( handle_error )
			dup2( root[ pipe_number ]->middlefd[ PIPE_WRITE ], STDERR_FILENO );
		close( root[ pipe_number ]->middlefd[ PIPE_WRITE ] );
	}

	void redirect_pipe_middle( Pipe_list* pipe_destination, bool handle_error )
	{
		if( root[ 0 ]->create_pipe )
		{
			printf( "Redirect pipe to input %x\n", root[ 0 ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
			dup2( root[ 0 ]->pipefd[ PIPE_READ ], STDIN_FILENO );
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
		}

		printf( "Redirect output to pipe %x\n", pipe_destination->root[ 0 ] );
		close( pipe_destination->root[ 0 ]->middlefd[ PIPE_READ ] );
		dup2( pipe_destination->root[ 0 ]->middlefd[ PIPE_WRITE ], STDOUT_FILENO );
		if( handle_error )
			dup2( pipe_destination->root[ 0 ]->middlefd[ PIPE_WRITE ], STDERR_FILENO );
		close( pipe_destination->root[ 0 ]->middlefd[ PIPE_WRITE ] );
	}

	void redirect_pipe_end( int sockfd, bool handle_error )
	{
		if( root[ 0 ]->create_pipe )
		{
			printf( "Redirect pipe to input %x\n", root[ 0 ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
			dup2( root[ 0 ]->pipefd[ PIPE_READ ], STDIN_FILENO );
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
		}
		
		dup2( sockfd, STDOUT_FILENO );
		if( handle_error )
			dup2( sockfd, STDERR_FILENO );	
	}

	void close_pipe_single()
	{
		if( root[ 0 ]->create_pipe )
		{
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
		}
	}

	void close_pipe_front( int pipe_number )
	{
		if( root[ 0 ]->create_pipe )
		{
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
		}

		printf( "Redirect middle to pipe %x\n", root[ pipe_number ] );
		close( root[ pipe_number ]->middlefd[ PIPE_WRITE ] );
		transfer_fd( root[ pipe_number ]->pipefd[ PIPE_WRITE ], root[ pipe_number ]->middlefd[ PIPE_READ ] );
		close( root[ pipe_number ]->middlefd[ PIPE_READ ] );

		root[ pipe_number ]->create_middle = false;
	}

	void close_pipe_front( Pipe_list* pipe_destination )
	{
		if( root[ 0 ]->create_pipe )
		{
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
		}

		printf( "Redirect middle to pipe %x\n", pipe_destination->root[ 0 ] );
		close( pipe_destination->root[ 0 ]->middlefd[ PIPE_WRITE ] );
		transfer_fd( pipe_destination->root[ 0 ]->pipefd[ PIPE_WRITE ], pipe_destination->root[ 0 ]->middlefd[ PIPE_READ ] );
		close( pipe_destination->root[ 0 ]->middlefd[ PIPE_READ ] );

		pipe_destination->root[ 0 ]->create_middle = false;
	}

	void close_pipe_middle( int pipe_number )
	{
		if( root[ 0 ]->create_pipe )
		{
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
		}

		printf( "Redirect middle to pipe %x\n", root[ pipe_number ] );
		close( root[ pipe_number ]->middlefd[ PIPE_WRITE ] );
		transfer_fd( root[ pipe_number ]->pipefd[ PIPE_WRITE ], root[ pipe_number ]->middlefd[ PIPE_READ ] );
		close( root[ pipe_number ]->middlefd[ PIPE_READ ] );

		root[ pipe_number ]->create_middle = false;
	}

	void close_pipe_middle( Pipe_list* pipe_destination )
	{
		if( root[ 0 ]->create_pipe )
		{
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
		}

		printf( "Redirect middle to pipe %x\n", pipe_destination->root[ 0 ] );
		close( pipe_destination->root[ 0 ]->middlefd[ PIPE_WRITE ] );
		transfer_fd( pipe_destination->root[ 0 ]->pipefd[ PIPE_WRITE ], pipe_destination->root[ 0 ]->middlefd[ PIPE_READ ] );
		close( pipe_destination->root[ 0 ]->middlefd[ PIPE_READ ] );

		pipe_destination->root[ 0 ]->create_middle = false;
	}
	
	void close_pipe_end()
	{
		if( root[ 0 ]->create_pipe )
		{
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
		}
	}

	void close_pipe()
	{
		if( root[ 0 ]->create_pipe )
		{
			close( root[ 0 ]->pipefd[ PIPE_WRITE ] );
			close( root[ 0 ]->pipefd[ PIPE_READ ] );
		}
		if( root[ 0 ]->create_middle )
		{
			close( root[ 0 ]->middlefd[ PIPE_WRITE ] );
			close( root[ 0 ]->middlefd[ PIPE_READ ] );
		}
		/*if( root[ 0 ]->create_error )
		{
			close( root[ 0 ]->errorfd[ PIPE_WRITE ] );
			close( root[ 0 ]->errorfd[ PIPE_READ ] );
		}*/
	}

	void delete_front()
	{
		root.front()->~Pipe();
		root.pop_front();
	}

	void clear_list()
	{
		while( !root.empty() )
			delete_front();
	}

	int pipe_size()
	{
		return root.size();
	}

	void pipe_show()
	{
		printf( "[SERV](PIPE) Start search exist pipe at %x\n", ( int* ) root[ 0 ] );
			
		if( root[ 0 ]->create_pipe )
			printf( "[SERV](PIPE) Find transfer pipe\n" );

		if( root[ 0 ]->create_middle )
			printf( "[SERV](PIPE) Find middle pipe\n" );
		/*
		if( root[ 0 ]->create_error )
			printf( "[SERV](PIPE) Find error pipe\n" );*/

		printf( "[SERV](PIPE) Search exist pipe done\n" );
	}

private:
	std::deque< Pipe* > root;
};

class Command_list
{
public:	
	Command_list( char* input, int sockfd, Pipe_list* pipe_list ) : sockfd( sockfd )
	{

		parse_command( input, pipe_list );
	}

	~Command_list()
	{
		//printf( "Command_list Distructor\n" );	
		free_list();
	}

	void free_list()
	{
		while( !root.empty() )
		{
			root.back()->~Command();
			root.pop_back();
		}

		pipe_vector[ 0 ]->~Pipe_list();
		pipe_vector.pop_front();
	}

	void parse_command( char* input, Pipe_list* pipe_list )
	{
		char temp[ 1000 ], tok[] = " \t\r\n", *point;

		pipe_vector.push_back( pipe_list );

		point = strtok( input, tok );
		while( point != NULL )
		{
			sscanf( point, "%s", temp );
			printf("[SERV](COMD) GET %s\n", temp );
			root.push_back( new Command( temp, strlen( temp ) ) );
			
			point = strtok( NULL, tok );
		}
	}

	COMMAND_RETURN exec()
	{
		if( root.empty() )
			return SUCCESS;
		char send[ MAXBUFFER ];
		if( strcmp( root.front()->command, "printenv" ) == 0 )
		{
			if( root.size() != 2 )
				return ERROR;	

			bzero( send, sizeof( send ) );
				
			char *printenv;
			printenv = getenv( root[ 1 ]->command );
					
			if( printenv != NULL )
			{
				strcpy( send, printenv );
				strcat( send, "\n" );

				printf( "[SERV](COMD) printenv %s", send );
				write( sockfd, send, sizeof( send ) );
			}
			else
				printf( "[SERV](COMD) NO RESULT!!\n" );
			//pipe_vector[ 0 ]->~Pipe_list();
			pipe_vector.pop_front();

			return SUCCESS;
		}
		else if( strcmp( root.front()->command, "setenv" ) == 0 )
		{
			if( root.size() != 3 )
				return ERROR;
			
			setenv( root[1]->command, root[2]->command, 1 );
			
			printf( "[SERV](COMD) setenv done\n" );
			//pipe_vector[ 0 ]->~Pipe_list();
			pipe_vector.pop_front();

			return SUCCESS;
		}
		else
		{
			PIPE_TYPE pipe_type = PIPE_SINGLE;
			
			while( !root.empty() )
			{
				if( strcmp( root.front()->command, "exit" ) == 0 )
				{
					printf( "[SERV](COMD) exit\n" );
					return EXIT;	
				}
				else
				{
					char **argv;
				
					printf( "[SERV](COMD) Parsing...\n" );
					
					int next_command = -1, arg_number = -1, pipe_number = 1;
					bool pipe_new_line = false;
					bool handle_error = false;
					bool handle_output_file = false;
					FILE *output;
					int outputfd;
					for( int i = 0; i < root.size() && next_command == -1; i++ )
						switch( root[ i ]->command[ 0 ] )
						{
							case '|':
								next_command = i;
								arg_number = i;
								if( sscanf( root[ i ]->command, "|%d", &pipe_number ) == 1 )
									pipe_new_line = true;

								pipe_type = pipe_type == PIPE_SINGLE ? PIPE_FRONT : PIPE_MIDDLE;
								break;
							case '!':
								next_command = i;
								arg_number = i;
								if( sscanf( root[ i ]->command, "!%d", &pipe_number ) == 1 )
									pipe_new_line = true;
								handle_error = true;
								
								pipe_type = pipe_type == PIPE_SINGLE ? PIPE_FRONT : PIPE_MIDDLE;
								break;
							case '>':
								if( i + 1 < root.size() )
								{
									arg_number = i;
									handle_output_file = true;
									output = fopen( root[ i + 1 ]->command, "w" );
									outputfd = fileno( output );
								}
								else
									printf( "[SERV](COMD) Output file format error\n" );

								break;
							default:
								break;
						}

					if( arg_number == -1 )
					{
						//printf( "[SERV](PIPE) no pipe exist\n" );
						argv = new char* [ root.size() ];
						for( int i = 0; i < root.size(); i++ )
						{
							argv[ i ] = root[ i ]->command;
							//printf( "[SERV](COMD) argv %d %s\n", i, argv[ i ] );
						}	
					}
					else
					{
						//printf( "[SERV](PIPE) find pipe exist\n" );
						argv = new char* [ arg_number ];
						for( int i = 0; i < arg_number; i++ )
						{
							argv[ i ] = root[ i ]->command;
							//printf( "[SERV](COMD) argv %d %s\n", i, argv[ i ] );
						}
					}

					printf( "[SERV](COMD) Executing...\n" );

					printf( "[SERV](COMD) Command is %s\n", pipe_type == PIPE_SINGLE ? "PIPE_SINGLE" : pipe_type == PIPE_FRONT ? "PIPE_FRONT" : pipe_type == PIPE_MIDDLE ? "PIPE_MIDDLE" : "PIPE_END" );

					//printf( "[SERV](COMD) Pipe number %d\n", pipe_number );
					if( pipe_new_line )
					{
						while( pipe_vector.size() <= pipe_number )
						{
							pipe_vector.push_back( new Pipe_list );
							printf( "Create pipe_list at %x\n", ( int* ) pipe_vector[ pipe_vector.size() - 1 ] );
						}

						pipe_vector[ pipe_number ]->check_pipe_number( pipe_number );
					}
					else
						pipe_vector[ 0 ]->check_pipe_number( pipe_number );
					//printf( "[SERV](PIPE) Create pipe to number %d\n", pipe_list->pipe_size() );
			
					//printf( "[SERV](PIPE) Prepare for pipe transfer...\n" );
					if( pipe_type == PIPE_FRONT || pipe_type == PIPE_MIDDLE )
						if( pipe_new_line )
							pipe_vector[ pipe_number ]->check_pipe( 0 );
						else
							pipe_vector[ 0 ]->check_pipe( pipe_number );
				
					signal( SIGCHLD, sig_chld_command );

					int stat;	
					pid_t exec = fork();	

					if( exec == 0 )
					{
						if( handle_output_file )
							printf( "handle_output_file is true\n" );
						
						switch( pipe_type )	
						{
						case PIPE_SINGLE:
							if( handle_output_file )
								pipe_vector[ 0 ]->redirect_pipe_single( outputfd, handle_error );
							else
								pipe_vector[ 0 ]->redirect_pipe_single( sockfd, handle_error );
							break;
						case PIPE_FRONT:
							if( handle_output_file )
								printf( "[SERV](COMD) ERROR pipe for output file\n" );
							else if( pipe_new_line )
								pipe_vector[ 0 ]->redirect_pipe_front( pipe_vector[ pipe_number ], handle_error );
							else
								pipe_vector[ 0 ]->redirect_pipe_front( pipe_number, handle_error );
							break;
						case PIPE_MIDDLE:
							if( handle_output_file )
								printf( "[SERV](COMD) ERROR pipe for output file\n" );
							else if( pipe_new_line )
								pipe_vector[ 0 ]->redirect_pipe_middle( pipe_vector[ pipe_number ], handle_error );
							else
								pipe_vector[ 0 ]->redirect_pipe_middle( pipe_number, handle_error );
							break;
						case PIPE_END:
							if( handle_output_file )
								pipe_vector[ 0 ]->redirect_pipe_end( outputfd, handle_error );
							else
								pipe_vector[ 0 ]->redirect_pipe_end( sockfd, handle_error );
							break;
						}

						if( execvp( root[ 0 ]->command, argv ) == -1 )
							printf( "Unknown command: [%s]\n", argv[ 0 ] );

						pipe_vector[ 0 ]->close_pipe();

						exit( EXIT_FAILURE );
					}
					else
					{
						switch( pipe_type )	
						{
						case PIPE_SINGLE:
							pipe_vector[ 0 ]->close_pipe_single();
							break;
						case PIPE_FRONT:
							if( pipe_new_line )
								pipe_vector[ pipe_number ]->close_pipe_front( 0 );
							else
								pipe_vector[ 0 ]->close_pipe_front( pipe_number );
							break;
						case PIPE_MIDDLE:
							if( pipe_new_line )
								pipe_vector[ pipe_number ]->close_pipe_middle( 0 );
							else
								pipe_vector[ 0 ]->close_pipe_middle( pipe_number );
							break;
						case PIPE_END:
							pipe_vector[ 0 ]->close_pipe_end();
							break;
						}
					
						//wait( &stat );
						//printf( "stat %d\n", stat );
					}
					waitpid( -1, &stat, 0 );

					printf( "[SERV](COMD) Execution done\n" );
					
					printf( "stat %d\n", stat );
					if( stat != EXIT_SUCCESS )
					{
						printf( "[SERV](COMD) ERROR command, Starting to throw all list\n" );
						
						pipe_vector[ 0 ]->clear_list();
						free_list();
					}
					else
					{
						pipe_vector[ 0 ]->delete_front();

						//printf( "[SERV](PIPE) There are %d pipe in vector\n", pipe_list->pipe_size() );

						if( next_command == -1 )
							free_list();
						else		
							for( int i = 0; i <= next_command; i++ )
							{
								root.front()->~Command();
								root.pop_front();
							}

						pipe_type = PIPE_END;
					}
				}
			}
		}
		//pipe_vector[ 0 ]->~Pipe_list();
		pipe_vector.pop_front();

		return SUCCESS;
	}
private:
    int sockfd;
	std::deque< Pipe_list* > pipe_vector;
	std::deque< Command* > root;
};

void RAS( int sockfd )
{
	char welcome_string[] = "****************************************\n** Welcome to the information server. **\n****************************************\n";
	char bash_symbol[] = "% ";
	char environment[ MAXBUFFER ] = "bin;.\0"; 
	char receive[ MAXBUFFER ] = "\0";
	//char send[ MAXBUFFER ];

	chdir( "/home/davis/RAS" );
	setenv( "PATH", "bin:.", 1 );
	setenv( "PWD", "/home/davis/RAS", 1 );
	
    Command_list command_list( receive, sockfd, new Pipe_list );
			
	printf( "%s", "[SERV](SEND) WELCOME STRING\n" );
	write( sockfd, welcome_string, strlen( welcome_string ) );
	write( sockfd, bash_symbol, strlen( bash_symbol ) );
	
	bzero( receive, MAXBUFFER );

	while( true )
	{
		int len;
		
		if( ( len = read( sockfd, receive, MAXBUFFER ) ) > 0 )
		{
			printf( "[SERV](RECV) %s", receive );
			
			command_list.parse_command( receive, new Pipe_list );
			COMMAND_RETURN command_return = command_list.exec();
			
			if( command_return == EXIT )
				return;
			
			command_list.free_list();
			
			write( sockfd, bash_symbol, strlen( bash_symbol ) );
			bzero( receive, MAXBUFFER );	
		}
		else if( len == 0 )
		{
			return;		
		}
	}
}

int main( int argc, char** argv )
{
	socklen_t           clientlen;
    int                 serverport;
	struct sockaddr_in  serveraddr, clientaddr;
    int                 sockfd, connectfd;
	pid_t               childpid;
	
	if( argc > 1 )
        serverport = atoi( argv[1] );
    else
        serverport = SERVERPORT;

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    bzero( &serveraddr, sizeof( serveraddr ) );
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );
    serveraddr.sin_port = htons( serverport );
    bind( sockfd, ( SA* ) &serveraddr, sizeof( serveraddr ) );
    listen( sockfd, MAXCLIENTS );
    
    signal( SIGCHLD, sig_chld_server );
	
	int num = 0;

    while( true )
    {
        clientlen = sizeof( clientaddr );
        connectfd = accept( sockfd, ( SA* ) &clientaddr, &clientlen);
    	
		printf( "[SERV] Client no.%i connected!\n", ++num );
        if( ( childpid = fork() ) == 0 )
        {
            close( sockfd );
            RAS( connectfd );
			close( connectfd );
			printf( "[SERV] Client no.%i terminated!\n", num );

            return 0;
        }
		else
		{
			close( connectfd ); 
		}
    }
    
    return 0;
}
//-----------------------------------------------

