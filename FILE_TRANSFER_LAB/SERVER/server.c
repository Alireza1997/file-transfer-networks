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
#define MAXBUFLEN 10000

//======== prototypes ========//
void processPacket(int *totalFrag,int *frag_no,int *size,	char *filename,	char *data, char *buf);
void writeToFile(int size,char *fileName, char *data);

//======== main ========//
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


	while (1){
	//listen 
		int listenStatus = listen(sockfd, BACKLOG);
		printf("\n **Listening for new messages** \n\n");  

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
		printf("message sent: %s \n\n", messageToSend);

		//======== proceed to file reception ========//
		if (!strcmp(messageToSend,"yes")){
			char newFileName[] = "";
			FILE *file;
			do{
				int totalFrag = 0, frag_no = 0, size = 0;
				char filename[100];
				char data[2000];	
				
				recieveBytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len);

				processPacket(&totalFrag, &frag_no, &size, filename,data,buf);
			
				messageToSend = "ACK";			
				sendBytes = sendto(sockfd, messageToSend, strlen(messageToSend),0,(struct sockaddr *)&their_addr, addr_len);
				//======== write data to file ========//		
				if (frag_no == 1){					
					strcat(newFileName,filename);
					file=fopen(newFileName,"wb");
					fwrite(data,size,1,file);
				}else{
					fwrite(data,size,1,file);
				}
	
				//writeToFile(file,size,filename,data);
				
				printf("totalFrag: %d \n", totalFrag);
				printf("frag_no: %d \n", frag_no);
				printf("size: %d \n", size);
				printf("filename: %s \n", filename);
				printf("newFileName: %s \n", newFileName);
				printf("recieveBytes: %d \n", recieveBytes);
				printf("sendBytes: %d \n", sendBytes);

				if (totalFrag == frag_no) break;
			}while(1);
			fclose(file);
		}
	}

	close(sockfd);
	return 0;
}

//======== helper functions ========//
void processPacket(int *totalFrag,int *frag_no,int *size, char *filename,	char *data , char *buf){
	int i, j = 0;
	for (i = 0; buf[i]!= ':'; i++){
		*totalFrag *=10;
		*totalFrag += buf[i] - '0';
	}
	for (i = i+1; buf[i]!= ':'; i++){
		*frag_no *=10;
		*frag_no += buf[i] - '0';
	}
	for (i = i+1; buf[i]!= ':'; i++){
		*size *=10;
		*size += buf[i] - '0';
	}j = i;
	for (i = i+1; buf[i]!= ':'; i++){
		filename[i-j-1] = buf[i]; 				
		//printf("%s \n", filename);
	}
	filename[i-j-1] = '\0';
	j = i;
	for (i = i+1; i-j <= *size; i++){
		data[i-j-1] = buf[i]; 				
		//printf("%u \n", buf[i]);
	}

}/*
void writeToFile(FILE *fileint, size,char *fileName,char *data){
	
	char newFileName[] = "copy-";
	strcat(newFileName,fileName);
	FILE *file=fopen(newFileName,"wb");
	fwrite(data,size,1,file);
	fclose(file);
	
}*/






















