#include        <sys/types.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <unistd.h>
#include        <errno.h>
#include        <string.h>
#include        <sys/socket.h>
#include        <netdb.h>
#include        <pthread.h>

#define SUCCESS "success"
#define FAILURE "failure"
#define NOUSERNAME "NOUSERNAME"
#define EXIT "exit"
#define WHO "who"
#define WRITETOALL "writetoall"
#define NEWUSER "newuser"
#define NOSPACE "nospace"

#define ANSI_COLOR_RED     "\x1b[2;31m"
#define ANSI_COLOR_GREEN   "\x1b[2;32m"
#define ANSI_COLOR_YELLOW  "\x1b[2;33m"
#define ANSI_COLOR_BLUE    "\x1b[2;34m"
#define ANSI_COLOR_MAGENTA "\x1b[2;35m"
#define ANSI_COLOR_CYAN    "\x1b[2;36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

struct user
{
	char * username;
	const char* color;
	int userSD; // User's SPECIFIC socket descriptor
};
typedef struct user * User;


char * commandFind(char* inputString)
{
	char * exitCommand = "@exit";
	char * whoCommand = "@who";

	if(strncmp(exitCommand,inputString,1) != 0)
	{
		return 0;
	}
	if(strstr(inputString, exitCommand) != NULL) return EXIT;
	else if(strstr(inputString, whoCommand) != NULL) return WHO;
	
	else 
	{
		return 0;
	}
}


User createNewUser(char* usernameString,int sd, const char* color)
{
	User newUserObject;
	newUserObject = (User)malloc(sizeof(User)); //Allocate
	newUserObject->username = usernameString;	
	newUserObject->userSD = sd; 
	newUserObject->color = (const char*)color;
	return newUserObject;
}


User userStructList[20];
int structListSize = 0;

int
claim_port( const char * port )
{
        struct addrinfo         addrinfo;
        struct addrinfo *       result;
        int                     sd;
        char                    message[256];
        int                     on = 1;

        addrinfo.ai_flags = AI_PASSIVE;         // for bind()
        addrinfo.ai_family = AF_INET;           // IPv4 only
        addrinfo.ai_socktype = SOCK_STREAM;     // Want TCP/IP
        addrinfo.ai_protocol = 0;               // Any protocol
        addrinfo.ai_canonname = NULL;
        addrinfo.ai_addrlen = 0;
        addrinfo.ai_addr = NULL;
        addrinfo.ai_canonname = NULL;
        addrinfo.ai_next = NULL;
        if ( getaddrinfo( 0, port, &addrinfo, &result ) != 0 )          // want port 3001
        {
                fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed errno is %s.  File %s line %d.\x1b[0m\n", port, strerror( errno ), __FILE__, __LINE__ );
                return -1;
        }
        else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
        {
                write( 1, message, sprintf( message, "socket() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
                freeaddrinfo( result );
                return -1;
        }
        else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
        {
                write( 1, message, sprintf( message, "setsockopt() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
                freeaddrinfo( result );
                close( sd );
                return -1;
        }
        else if ( bind( sd, result->ai_addr, result->ai_addrlen ) == -1 )
        {
                freeaddrinfo( result );
                close( sd );
                write( 1, message, sprintf( message, "\x1b[2;33mBinding to port %s ...\x1b[0m\n", port ) );
                return -1;
        }
        else if ( listen( sd, 100 ) == -1 )
        {
                printf( "listen() failed in file %s line %d\n", __FILE__, __LINE__ );
                close( sd );
                return 0;
        }
        else
        {
                write( 1, message, sprintf( message,  "\x1b[1;32mSUCCESS : Bind to port %s\x1b[0m\n", port ) );
                freeaddrinfo( result );
                return sd;                      // bind() succeeded;
        }
}


void chathistoryAdd(char* chatString)
{
	FILE *fp;
	fp = fopen("chathistory.txt", "a");
	fprintf(fp, "%s\r\n",chatString);
	fclose(fp);
}
	

void printList() // Function to see whats inside database
{
	int i;
	printf("Chatroom List_________\n");
	for(i = 0;i<structListSize;i++)
	{
		printf("\x1b[2;32;41m(PL)USER: %s,\x1b[0;0m",userStructList[i]->username);
	}
	printf("\n");
}

char * returnList(char*buffer) //Used for @who
{
	int i;
	buffer = (char*) malloc(sizeof(char)*1+strlen(userStructList[i]->username)+1);
	for(i = 0;i<structListSize;i++)
	{
		strcat(buffer,userStructList[i]->username);
		strcat(buffer," ");
		buffer = (char*)realloc(buffer,1 + strlen(userStructList[i]->username)+1);
	}
	return buffer;
}

int usernameCheck(char* username)
{
	int i;
	printf("Checking New User____________\n");
	if(strlen(username)<=0)		return 0; 		//Empty username
	if(strlen(username) > 100) return 0; 		//Username too long
	if(structListSize == 0) return 1;  			//list has no elements to check, good to go
	if(structListSize >=20)						//List has max memebers
	{
		printf("Not enough space");
		return -1;
	}
	
	for(i = 0; i <= structListSize-1; i++) //Check every username
	{
		printf("CHECKING:%s to %s\n",username,userStructList[i]->username);
		if(strcmp(username,userStructList[i]->username) == 0) return 0; //Taken
	}
	printf("\n");
	return 1;
}


int usernameAdd(User incomingStruct) // NOTE: usernameCheck() ran before this
{
	userStructList[structListSize] = incomingStruct;
	structListSize++;
	return 1;
}


int usernameRemove(User  incomingStruct)
{
	int i;
	int existingUser = 0;

	for(i = 0;i<structListSize;i++)
	{
		if(strcmp(userStructList[i]->username,incomingStruct->username) == 0)
		{
			/* Note dont need to free color b/c its a macro. Dont need to free userSD b/c its int*/
			existingUser = 1;
			free(&userStructList[i]->username);
			printList();
			if(structListSize != 0) // Replace struct with the latest struct.
			{
				userStructList[i] = userStructList[structListSize-1];
				structListSize -= 1;
			}
			return 1;
		}
	}	
	if (existingUser == 0)
	{
		printf("user not found\n");
		return 0;
	}		
}

int colorPickerNumber = 0;
const char * colorPicker()
{
	const char *color;
	int i;
	if(colorPickerNumber > 5)colorPickerNumber = 0;
	if(colorPickerNumber == 0)color=ANSI_COLOR_RED;
	if(colorPickerNumber == 1)color=ANSI_COLOR_GREEN;
	if(colorPickerNumber == 2)color=ANSI_COLOR_YELLOW;
	if(colorPickerNumber == 3)color=ANSI_COLOR_BLUE;
	if(colorPickerNumber == 4)color=ANSI_COLOR_MAGENTA;
	if(colorPickerNumber == 5)color=ANSI_COLOR_CYAN;
	colorPickerNumber++;
	return color;
}


void writeToAllUsers(User userRequestStruct, char* buffer, char * status, char *request)
{
	/* Function that makes easier to write to all memebers */
	/*Create/edit the buffer*/
	int i;
	if (strcmp(status,EXIT) == 0) sprintf(buffer,"\x1b[2;32;41mUser: %s disconnected\x1b[0m\n",userRequestStruct->username);
	else if(strcmp(status,WRITETOALL) == 0) sprintf(buffer,"%s%s:%s\x1b[0m",userRequestStruct->color, userRequestStruct->username ,request); //make text to send back
	else return;
	
	/*Send out buffer */
	printf("%s\n",buffer);
	for(i = 0; i<= structListSize-1; i++)
	{
		write(userStructList[i]->userSD,buffer,strlen( buffer ));
	}
	memset(buffer,0,sizeof(buffer)); // Refresh buffer for next message	
	
}


void *
client_session_thread( void * arg )
{
        int                     sd;
        char                    request[2048];
        char                    temp;
        int                     i;
        int                     limit, size;
		char					username[512];
		char					buffer[2048];
		char					chatHistory[2048];
		char					*whoCommandBuffer;
		const char*	colorTest;
		User userRequestStruct;
		int clientContinue = 1;

        sd = *(int *)arg;
        free( arg );                                    // keeping to memory management covenant
        pthread_detach( pthread_self() );               // Don't join on this thread
		
		pthread_mutex_t lock;
		pthread_mutex_init(&lock,0);

		
		/*               Checking Username               */
		read(sd,username, sizeof(username)); // GOT USERNAME FROM CLIENT
		printf("User |%s| connecting, checking\n",username);
		if(usernameCheck(username) == 0)
		{
			printf("Username: |%s| invalid(empty,exisiting,size), disconnecting user...\n",username);
			write(sd,FAILURE,strlen(FAILURE)+1); //Send back fail
			pthread_mutex_destroy(&lock); 
			close(sd);
			return 0;
		}
		else if(usernameCheck(username) == -1)
		{
			printf("Chatroom full, disconnecting user\n");
			write(sd,NOSPACE,strlen(NOSPACE)+1); //Send back fail
			pthread_mutex_destroy(&lock); 
			close(sd);
			return 0;
		}
		else 
		{
			colorTest = (char*)colorPicker();
			userRequestStruct = createNewUser(username,sd,colorTest);
			usernameAdd(userRequestStruct);  // Add Username + SD into database, else too much space
			printList(); 
			printf("Connecting User: %s\n", userRequestStruct->username );
			write(sd,SUCCESS,strlen(SUCCESS)+1);
		}
		/*               Checking Username               */

		
		/* 				 New User Joined				 */
		writeToAllUsers(userRequestStruct, buffer, NEWUSER ,request);
		/* 				 New User Joined				 */
		
		
		/* 				 Reading from clients			 */
        while (clientContinue)
        {
			if(read( sd, request, sizeof(request)) == 0 )		// Got message from client
			{
				fprintf( stderr, "\x1b[1;31mReading failed, User: %s Disconnect \x1b[0m\n",userRequestStruct->username);
				usernameRemove(userRequestStruct);
				pthread_mutex_destroy(&lock); 
				close( sd );
				return 0;
			}
			if(commandFind(request) == EXIT)  					//tell everyone this person left
			{
				writeToAllUsers(userRequestStruct, buffer, EXIT,request);
				clientContinue = 0;
			}
			else if(commandFind(request) == WHO)				//Send back list of all memebers
			{
				whoCommandBuffer = returnList(whoCommandBuffer);
				write(userRequestStruct->userSD,whoCommandBuffer,strlen(whoCommandBuffer));
				free(whoCommandBuffer);
			}
			else												// Send Back messages
			{	
				sprintf(chatHistory,"%s:%s", username,request); //remove color for chat history
				chathistoryAdd(chatHistory);
				writeToAllUsers(userRequestStruct,buffer,WRITETOALL, request);
				/*         Sending Messages to all SD			*/
			}
        }
		
		printf("User %s disconnect...\n",userRequestStruct->username);
		usernameRemove(userRequestStruct);
		pthread_mutex_destroy(&lock); 
        close( sd );
        return 0;
}

int
main( int argc, char ** argv )
{
        int                     sd;
        char                    message[256];
        pthread_t               tid;
        socklen_t               ic;
        int                     fd;
        struct sockaddr_in      senderAddr;
        int *                   fdptr;

        if ( (sd = claim_port( "3001" )) == -1 ) //User trying to claim port
        {
                write( 1, message, sprintf( message,  "\x1b[1;31mCould not bind to port %s errno %s\x1b[0m\n", "3001", strerror( errno ) ) );
                return 1;
        }
        else
        {
                while (1)
                {
					    ic = sizeof(senderAddr);
						fd = accept( sd, (struct sockaddr *)&senderAddr, &ic );
                        fdptr = (int *)malloc( sizeof(int) );
                        *fdptr = fd;                                    // pointers are not the same size as ints any more.
                        if ( pthread_create( &tid, 0, client_session_thread, fdptr ) != 0 )
                        {
                                printf( "pthread_create() failed in file %s line %d\n", __FILE__, __LINE__ );
                                return 0;
                        }
                        else
                        {
                                continue;
                        }
                }
                printf( "No longer accepting incoming connections file %s line %d\n", __FILE__, __LINE__ );
                close( sd );
                return 0;
        }
}