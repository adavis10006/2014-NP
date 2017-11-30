debug_welcome();
printf( "%s", "[SERV](SEND) WELCOME STRING\n" );

debug_receive(receive)
printf( "[SERV](RECV) %s", receive );


debug_client_connect();
		printf( "[SERV] Client no.%i connected!\n", ++num );
        
		debug_client_terminate();
			printf("[SERV] Client no.%i terminated!\n", num);
			
debug_fork_information(getpid(), childpid);
			