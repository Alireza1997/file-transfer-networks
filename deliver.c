/*
** talker.c -- a datagram "client" demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define SERVERPORT "4950" // the port users will be connecting to
#define MAXBUFLEN 100
int main(int argc, char *argv[])
{
	printf ("No of arguments is %d\n",argc);
	printf ("Server address =%s\n",argv[1]);
	printf ("Server port number=%s\n",argv[2]);

	char commandName [100];
	char fileName [100];
	scanf("%s", commandName);
	scanf("%s", fileName);
	printf("command name recieved: %s \n", commandName);
	printf("file name recieved: %s \n", fileName);

	char buf[MAXBUFLEN];
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_storage recieving_addr;
	socklen_t addr_length;
	addr_length=sizeof recieving_addr;	

	memset(&hints, 0, sizeof hints);
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;


	if(strcmp (commandName, "ftp")!=0){
		printf("Usage: ftp <filename>\n");
		exit (1);

	}
	FILE *file=fopen(fileName,"r");
	if (!file) {
		printf ("File does not exist\n");
		exit(1);
	}

	int getAddressStatus= getaddrinfo(argv[1],argv[2], &hints, &res);
	printf("getAddrInfoStatus: %d \n", getAddressStatus);

	int sockDescriptor=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	printf("socket descriptor: %d \n", sockDescriptor);

	char* messageToSend = "ftp";
	int sendBytes= sendto(sockDescriptor, messageToSend, strlen(messageToSend),0,res->ai_addr, res->ai_addrlen);
	printf("sendBytes: %d \n", sendBytes);
	printf("message sent: %s \n", messageToSend); 

	int recieveBytes=recvfrom(sockDescriptor, buf,sizeof (buf) , 0, (struct sockaddr *)&recieving_addr, &addr_length);
	buf[recieveBytes] = '\0';
	printf("recieveBytes: %d \n", recieveBytes);
	printf("message recieved: %s \n", buf); 

	if (!strcmp(buf,"yes")){
		printf("A file transfer can start \n");
	}

}
