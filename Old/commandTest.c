#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<string.h>


int clientContinue = 1;




int commandFind(char* inputString)
{
	char * exitCommand = "@exit";
	int ret;
	char *buffer;
	//buffer = (char*) malloc(sizeof(char)* strlen(inputString));
	
	//strcpy(buffer,inputString);
	
	
	//printf("Buffer:%s\n",buffer);
	printf("EXIT:%s\n",exitCommand);
	//ret = (int*) strncmp(exitCommand,inputString,1);
	//printf("ret:%d\n",ret);
	if(strncmp(exitCommand,inputString,1) != 0)
	{
		printf("Not a command (@command)\n");
		return 0;
	}
	
	printf("__\n");
	if(strstr(inputString, exitCommand) != NULL) 
	{
		
		printf("Found command\n");
		return 1;
	}
	else 
	{
		printf("Not a command (@command)\n");
		return 0;
	}
	printf("_2_\n");

	
	
}



int main()
{
	char usernameInput[512];
	if(fgets(usernameInput,60,stdin) != NULL)
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
	
	
	
	
	
}









