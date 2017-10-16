/*
** server.c -- a datagram sockets "server" demo
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

#define MYPORT "4950"   // the port users will be connecting to
#define BACKLOG 10
#define MAXBUFLEN 100


int main(int argc, char* argv[]){

	char* UDP_listen_port = argv[1];
	printf ("No of arguments is %d\n",argc);
	printf ("Server port number=%s\n",UDP_listen_port);

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	addr_len = sizeof their_addr;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM; //UDP
	hints.ai_flags = AI_PASSIVE; // use my IP

	
//get address info	
	if ((rv = getaddrinfo(NULL, UDP_listen_port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
//open udp socket
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

//bind to a specific port
	int bindStatus = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);

//listen 
	int listenStatus = listen(sockfd, BACKLOG);

//accept and communicate

	int recieveBytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len);
	buf[recieveBytes] = '\0';	

	char* messageToSend = "";
	if (!strcmp(buf,"ftp")){
		messageToSend = "yes";		
	}else{
		messageToSend = "no";
	}
	int sendBytes = sendto(sockfd, messageToSend, strlen(messageToSend),0,(struct sockaddr *)&their_addr, addr_len);

	printf("getAddrInfoStatus: %d \n", rv);
	printf("socket descriptor: %d \n", sockfd);
	printf("bindStatus: %d \n", bindStatus);
	printf("listenStatus: %d \n", listenStatus);
	printf("recieveBytes: %d \n", recieveBytes);
	printf("sendBytes: %d \n", sendBytes);
	printf("message recieved: %s \n", buf); 
	printf("message sent: %s \n", messageToSend); 


	close(sockfd);
	return 0;
}
