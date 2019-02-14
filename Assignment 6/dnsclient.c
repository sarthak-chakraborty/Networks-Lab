#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXLINE 100

void main(){

	int	sockfd ;
	struct sockaddr_in	serv_addr;
	int i, n, flag = 0;
	socklen_t len;
	char buff[MAXLINE];
	char c;

	// Creating the socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family	= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port	= htons(6660);
	
	printf("Enter host name: ");		// Host name is taken as input
	scanf("%s",buff);

	sendto(sockfd, (const char *)buff, strlen(buff)+1, 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
	
	// Receiving the IP addresses from server
	printf("IP ADDRESS: \n");
	while(recvfrom(sockfd, (const char *)buff, MAXLINE, 0, (const struct sockaddr *)&serv_addr, &len)){
		flag = 1;
		printf("%s\n", buff);
	}
	
	if(flag == 0)
		printf("Domain Name Incorrect\n");
	
	close(sockfd);
}


