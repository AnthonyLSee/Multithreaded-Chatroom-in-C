#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<string.h>

#include "server.h"

int clientContinue = 1;


char * commandFind(char* inputString)
{
	char * exitCommand = "@exit";

	if(strncmp(exitCommand,inputString,1) != 0)		return 0;
	if(strstr(inputString, exitCommand) != NULL) return EXIT;

	return 0;
}

int
connect_to_server( const char * server, const char * port )
{
	int			sd;
	struct addrinfo		addrinfo;
	struct addrinfo *	result;
	char			message[256];

	addrinfo.ai_flags = 0;
	addrinfo.ai_family = AF_INET;		// IPv4 only
	addrinfo.ai_socktype = SOCK_STREAM;	// Want TCP/IP
	addrinfo.ai_protocol = 0;		// Any protocol
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;
	if ( getaddrinfo( server, port, &addrinfo, &result ) != 0 )
	{
		fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed.  File %s line %d.\x1b[0m\n", server, __FILE__, __LINE__ );
		return -1;
	}
	else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
	{
		freeaddrinfo( result );
		return -1;
	}
	else
	{
		do {
			if ( errno = 0, connect( sd, result->ai_addr, result->ai_addrlen ) == -1 )
			{
				sleep( 1 );
				write( 1, message, sprintf( message, "\x1b[2;33mConnecting to server %s ...\x1b[0m\n", server ) );
			}
			else
			{
				freeaddrinfo( result );
				return sd;		// connect() succeeded
			}
		} while ( errno == ECONNREFUSED );
		freeaddrinfo( result );
		return -1;
	}
}


int read_chat_thread(void *arg)
{
	char			buffer[512];
	int 			sd;
	sd = *(int *)arg;
	printf("Now Reading...\n");
	while(clientContinue)
	{
		if(read( sd, buffer, sizeof(buffer)) == 0 )	// Got message from server / other clients
		{
			fprintf( stderr, "\x1b[1;31mServer Disconnect, press enter to exit\x1b[0m\n");
			clientContinue = 0;
		}
		printf("%s\n",buffer);						// print said message
		memset(buffer,0,sizeof(buffer));			// clear buffer for next string
	}
	close(sd);
	return 0;
	
}



char * checkUsername(char* user,char* usernameCheckStatus)
{
	if(strcmp(usernameCheckStatus,FAILURE) == 0) 		// USERNAME EXISTS
	{
		printf("Validation failed, diconnecting...\n\n");
		return FAILURE;
	}
	else if(strcmp(usernameCheckStatus,NOSPACE) == 0)	// CHATROOM FULL
	{
		printf("Chatroom is full, disconnecting...\n\n");
		return NOSPACE;
	}
	
	else return SUCCESS;
		
}


int
main( int argc, char ** argv)
{
	int				sd;
	char			message[256];
	char			string[512];
	char			buffer[512];
	char			bufferB[512];
	char			prompt[] = "Enter a string>>";
	char			output[512];
	int				len;
	int				count;
	char 			c;
	pthread_t       tid;
	int *			sdptr; // Socket Desc. Pointer

	char usernameInput[512];
	char usernameCheckStatus[512];
	
	/*               Getting Username               */
	printf("\n*=============================*\n Chatroom %s\n*=============================*\n",argv[1]);
	printf("Please enter a username that is less than 100 characters and is not 0 either\n");
	printf("Enter username:");
	if(fgets(usernameInput,102,stdin) != NULL) // Note the 102. username has max 100, server will see if over 100
	{
		/* cleanup the \n at the end of array from fgets() */
		char *pos;
      	if ((pos = strchr(usernameInput, '\n')) != NULL) 
      	{
        	*pos = '\0';
      	}	
      	else
      	{
      		while ((c = getchar()) != EOF && c != '\n');
   		}
	}
	/*               Getting Username               */
	
	
	if ( argc < 2 )
	{
		fprintf( stderr, "\x1b[1;31mNo host name specified.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
		exit( 1 );
	}
	else if ( (sd = connect_to_server( argv[1], "3001" )) == -1 )
	{
		write( 1, message, sprintf( message,  "\x1b[1;31mCould not connect to server %s errno %s\x1b[0m\n", argv[1], strerror( errno ) ) );
		return 1;
	}
	else
	{
		printf("Username:%s\n",usernameInput);
		printf("Connection Sucessful, validating username...\n");
		
		
		/*         Sending Username + Validation        */
		if(write( sd, usernameInput, strlen( usernameInput )+1) == -1) return 0; // Write To server
		read(sd,usernameCheckStatus,sizeof(usernameCheckStatus)); 				// Get validation from server
		
		if(checkUsername(usernameInput,usernameCheckStatus) != SUCCESS) // Runs through external function
		{
			close(sd);
			return 0;
		}
		else
		{
			printf("Username created, connecting...\n");
			printf("\n=================== Welcome %s ===================\n\n",usernameInput);
		}
		/*         Sending Username + Validation        */

		
		/*			The Chatroom (Reading)				*/
		sdptr = (int *)malloc( sizeof(int) );
		*sdptr = sd; 
        if ( pthread_create( &tid, 0, read_chat_thread, sdptr ) != 0 )
        {
            printf( "pthread_create() failed in file %s line %d\n", __FILE__, __LINE__ );
			return 0;
        }
		/*			The Chatroom (Reading)				*/
		
		
		/*			The Chatroom (Writing)				*/
		while (clientContinue)
		{
			if(fgets(string,60,stdin) != NULL)
			{
				/* cleanup the \n at the end of array from fgets() */
				char *pos;
				if ((pos = strchr(string, '\n')) != NULL) 
				{
					*pos = '\0';
				}	
				else
				{
				while ((c = getchar()) != EOF && c != '\n');
				}
			}
			
			if(commandFind(string) == EXIT) clientContinue = 0;
			else
			{
				write( sd, string, strlen( string ) + 1 ); 	// Send message to server
			}
		}
		/*			The Chatroom (Writing)				*/

		close( sd );
		return 0;
	}
}














