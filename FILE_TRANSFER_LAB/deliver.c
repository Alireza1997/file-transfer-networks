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
#include <time.h>

#define SERVERPORT "4950" // the port users will be connecting to
#define MAXBUFLEN 100

//======== prototypes ========//
struct packet {
	unsigned int total_frag; 
	unsigned int frag_no; 
	unsigned int size;
	char* filename;
	char filedata[1000];
};



int getFileSize(FILE *file);
char* readFile(FILE *file, unsigned char* buffer);
char* processPacket(struct packet pack, int* length, char* packetInfo);


//======== main ========//
int main(int argc, char *argv[])
{
	//======== initialization and input validation ========//
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
	clock_t start_t, end_t;	

	memset(&hints, 0, sizeof hints);
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;

	int fileSize;
	int totalFrag;
 
	struct packet packets[300];
	

	//======== file processing ========//
	if(strcmp (commandName, "ftp")!=0){
		printf("Usage: ftp <filename>\n");
		exit (1);

	}
	FILE *file=fopen(fileName,"rb");
	if (!file) {
		printf ("File does not exist\n");
		exit(1);
	}

	fileSize = getFileSize(file);
	printf ("fileSize: %d\n" , fileSize);
	totalFrag = (int)((fileSize-1)/1000.0)+1;
	printf ("totalFrag: %d\n" , totalFrag);

	for (int i = 0; i < totalFrag; i++){
		packets[i].total_frag = totalFrag;
		packets[i].frag_no = i+1;
		if (i != totalFrag-1)
			packets[i].size = 1000;
		else
			packets[i].size = fileSize - (totalFrag-1)*1000;
		packets[i].filename = fileName;
		readFile(file, packets[i].filedata);
		
		printf("packet: %d\n", i);
		printf("frag_no %d\n", packets[i].frag_no);
		printf("size %d\n", packets[i].size);
		printf("filename %s\n\n", packets[i].filename);		
		//for(int j = 0; j<1000; j++)
		//	printf("%u ", packets[i].filedata[j]);
		
	}


	//======== message transfer ========//
	int getAddressStatus= getaddrinfo(argv[1],argv[2], &hints, &res);


	int sockDescriptor=socket(res->ai_family,res->ai_socktype,res->ai_protocol);


	char* messageToSend = "ftp";
	start_t = clock(); //start roundtrip timer
	int sendBytes= sendto(sockDescriptor, messageToSend, strlen(messageToSend),0,res->ai_addr, res->ai_addrlen);
	

	int recieveBytes=recvfrom(sockDescriptor, buf,sizeof (buf) , 0, (struct sockaddr *)&recieving_addr, &addr_length);
	end_t = clock(); //end of roundtrip timer

	printf("start_t: %ld \n", start_t);
	printf("end_t: %ld \n", end_t);
	printf("getAddrInfoStatus: %d \n", getAddressStatus);
	printf("socket descriptor: %d \n", sockDescriptor);
	printf("sendBytes: %d \n", sendBytes);
	printf("message sent: %s \n", messageToSend); 
	buf[recieveBytes] = '\0';
	printf("recieveBytes: %d \n", recieveBytes);
	printf("message recieved: %s \n", buf);
	printf("round trip time: %f\n", (double)(end_t - start_t)/CLOCKS_PER_SEC); 

	if (!strcmp(buf,"yes")){
		printf("A file transfer can start \n");
	}

		
	//======== file transfer ========//
	for (int i = 0; i < totalFrag; i++){
		int length;
		char packetInfo[3000] = "";
		processPacket(packets[i],&length,packetInfo);
		printf ("%d %d %s\n", length, packets[i].size, packetInfo);
		length = length + packets[i].size;

		sendBytes= sendto(sockDescriptor, packetInfo, length ,0,res->ai_addr, res->ai_addrlen);
		printf("sendBytes: %d \n", sendBytes);

		//======== ACK handling ========//

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1;
		setsockopt(sockDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
		
		int recieveBytes=recvfrom(sockDescriptor, buf,sizeof (buf) , 0, (struct sockaddr *)&recieving_addr, &addr_length);
		buf[recieveBytes] = '\0';

		if (recieveBytes >= 0 && strcmp(buf,"ACK") >= 0){
			printf("recieveBytes: %d \n", recieveBytes);
			printf("message recieved: %s \n", buf);
		}else{
			printf("ACK not recieved");
			i--;
		}
	}	



	printf ("hi\n");




	fclose(file);

}


//======== helper functions ========//
char* readFile(FILE *file, unsigned char* buffer){	
	fread(buffer,1000,1,file);
	//for(int i = 0; i<1000; i++)
    	//printf("%u ", buffer[i]);
	return buffer;
}

int getFileSize(FILE *file){
	fseek(file, 0L, SEEK_END);
	int fileSize = ftell(file);
	rewind(file);
	return fileSize;
}

char* processPacket(struct packet pack, int* length , char* packetInfo){

	char buffer [200];
	sprintf(buffer,"%d:",pack.total_frag);
	strcat(packetInfo, buffer);


	sprintf(buffer,"%d:",pack.frag_no);
	strcat(packetInfo, buffer);

 
	sprintf(buffer,"%d:",pack.size);
	strcat(packetInfo, buffer);

	strcat(packetInfo, pack.filename);	
	strcat(packetInfo, ":");

	*length = strlen(packetInfo);	

	for (int i = 0; i < pack.size; i++){
		//sprintf(buffer,"%c",pack.filedata[i]);
		//strcat(packetInfo, buffer);
		packetInfo[*length + i] = pack.filedata[i];
	}


	return packetInfo;

}
























