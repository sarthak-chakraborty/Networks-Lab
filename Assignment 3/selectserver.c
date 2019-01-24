#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#define MAXLINE 100
#define PACKET 50

// Return max of two integers
int max(int a, int b){
	return (a>b)?a:b;
}

void main(){
	int sockfd1, sockfd2, newsockfd;
	int	clilen;
	int nfds, r;
	int i, j, flag, fd, rret, n;
	struct sockaddr_in	cli_addr, serv_addr;
	struct hostent *ip;
	struct in_addr **adr;
	fd_set readfs;
	char buff[MAXLINE];
	char file[MAXLINE];

	// Creating socket for TCP client
	if((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Cannot create socket\n");
		exit(0);
	}
	// Creating socket for UDP client
	if((sockfd2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(6660);

	// Binding socket
	if(bind(sockfd1, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Failed to bind local address\n");
		exit(0);
	}
	if(bind(sockfd2, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Unable to bind local address\n");
		exit(0);
	}

	printf("Server Running...\n");
	listen(sockfd1, 5);

	while(1){

		// Selecting among two sockets
		FD_ZERO(&readfs);
		FD_SET(sockfd1, &readfs);
		FD_SET(sockfd2, &readfs);
		nfds = max(sockfd1, sockfd2) + 1;
		r = select(nfds, &readfs, 0, 0, 0);

		if(r == -1){
			perror("Error in selecting socket\n");
			exit(0);
		}

		// TCP socket is selected
		if(FD_ISSET(sockfd1, &readfs)){
			memset(buff, 0, sizeof(buff));

			clilen = sizeof(cli_addr);
			newsockfd = accept(sockfd1, (struct sockaddr *)&cli_addr, &clilen);

			if(newsockfd < 0){
				perror("Accept error\n");
				exit(0);
			}

			// Fork to create a child process
			if(fork() == 0){	
				close(sockfd1);

				recv(newsockfd, buff, PACKET, 0);	// Receiving the request to send Bag of Words
				if(!strcmp(buff, "Request BOW")){
					fd = fopen("word.txt", "r");
					memset(buff, 0, sizeof(buff));

					// If file is found
					if(fd > 0){
						printf("MESSAGE: \n");
						while(fscanf(fd, "%s", buff) == 1){		// Scan the file line by line
							printf("%s\n", buff);
							send(newsockfd, buff, strlen(buff)+1, 0);	// Sending the word to client
						}
						strcpy(buff, "\0");		// Sending a null string to denote the end of BOW
						send(newsockfd, buff, PACKET, 0);
					}
				}	
				close(newsockfd);
				exit(0);
			}
			close(newsockfd);
			
		}

		// UDP socket is connected
		if(FD_ISSET(sockfd2, &readfs)){

			memset(buff, 0, sizeof(buff));

			// Forking to create child project
			if(fork() == 0){
				clilen = sizeof(cli_addr);
				n = recvfrom(sockfd2, (const char *)buff, MAXLINE, 0, (const struct sockaddr *)&cli_addr, &clilen);		// Receiving the host name
				printf("RECEIVED FROM DNS CLIENT: %s\n", buff);

				ip = gethostbyname(buff);
				if(ip != NULL){			
					adr = (struct in_addr **)ip->h_addr_list;

					// Sending the IP address to the client
					for(i=0; adr[i] != NULL; i++){
						strcpy(buff, inet_ntoa(*adr[i]));
						sendto(sockfd2, (const char *)buff, strlen(buff)+1, 0, (const struct sockaddr *)&cli_addr, sizeof(cli_addr));
						printf("SENT TO DNS CLIENT: %s\n", inet_ntoa(*adr[i]));
					}
				}
				sendto(sockfd2, (const char *)buff, 0, 0, (const struct sockaddr *)&cli_addr, sizeof(cli_addr));	// Sending a 0 length string to denote the end of IP addresses.
				exit(0);
			}
		}
	}
}