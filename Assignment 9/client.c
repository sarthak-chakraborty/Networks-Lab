#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define MAXSIZE 100

int main(){
	int sockfd;
	struct sockaddr_in servaddr;
	int socklen;
	char buff[MAXSIZE];
	
	// Socket creation
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("Socket creation failed\n");
		exit(1);
	}
	
	// Initialize servaddr
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(6666);
	socklen = sizeof(servaddr);
	
	printf("Enter string: \n");
	gets(buff);
	
	// Send the string
	sendto(sockfd, buff, MAXSIZE, 0, (struct sockaddr *)&servaddr, socklen);
	printf("MESSAGE SENT TO SERVER\n\n");
	
	// On a blocking call for recvfrom
	int n = recvfrom(sockfd, buff, MAXSIZE, 0, (struct sockaddr *)&servaddr, &socklen);
	printf("ECHO FROM SERVER:\n");
	printf("%s\n",buff);
	
	close(sockfd);
	return 0;
}
