#ifndef __CLIENTTEST_H
#define __CLIENTTEST_H

#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<string.h>



#define SUCCESS "success"
#define FAILURE "failure"
#define NOUSERNAME "NOUSERNAME"
#define EXIT "exit"
#define WHO "who"
#define WRITETOALL "writetoall"
#define NEWUSER "newuser"
#define NOSPACE "nospace"


struct user
{
	char * username;
	int userSD; // User's SPECIFIC socket descriptor
};
typedef struct user * User;


#endif