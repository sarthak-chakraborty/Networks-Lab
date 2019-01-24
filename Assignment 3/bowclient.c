#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h> 
#define MAX 100

void main(){

	int sockfd;
	int clilen;
	struct sockaddr_in servaddr;
	char buff[MAX];
	char c, prev = '\0';
	int i, fd, w_count = 0;

	// Creating the socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket crearion failed.\n");
		exit(0);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(6660);

	// Connecting the socket with the server
	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Unable to connect to server\n");
		exit(0);
	}

	strcpy(buff, "Request BOW");
	send(sockfd, buff, strlen(buff)+1, 0);	// Request for the bag of words

	// Counting the number of words
	while(recv(sockfd, &c, 1, 0)){
		if(c=='\0' && prev=='\0')
			break;
		if(c=='\0')
			w_count++;
		prev=c;
	}
	
	printf("Words Transfer done\n");
	printf("Words Count: %d\n", w_count);	// Printing the number of words

	close(sockfd);

}