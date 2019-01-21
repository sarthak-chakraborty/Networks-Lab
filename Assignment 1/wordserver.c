#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#define MAXLINE 1024


int main(){
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	// Creating the socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sockfd < 0){
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(6666);

	// Binding th socket
	if(bind(sockfd,(const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Binding Failed");
		exit(EXIT_FAILURE);
	}

	printf("\nServer Running...\n");

	// Server is always running
	while(1){
		int n, flag = 0;
		socklen_t len;
		char buffer[MAXLINE];
		len = sizeof(cliaddr);

		n = recvfrom(sockfd, (const char *)buffer, MAXLINE, 0, (const struct sockaddr *)&cliaddr, &len);	// Receiving the filename
		buffer[n] = '\0';
		printf("FILENAME: %s\n",buffer);
		FILE* f = fopen(buffer,"r");

		if(f == NULL){
			char *mssg = "NOTFOUND";
			sendto(sockfd, (const char *)mssg, strlen(mssg), 0, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));	// If file is not there, send error message
			continue;
		}
		else{
			char mssg[MAXLINE]; 
			fscanf(f, "%s", mssg);
			sendto(sockfd, (const char *)mssg, strlen(mssg), 0, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
			printf("Sent %s\n", mssg);	// If file is found, scan the first word "HELLO" in the file and send to client
		}

		do{		// Scan the other words and send to the client until the client wants to
			char words[MAXLINE];
			len = sizeof(cliaddr);
			n = recvfrom(sockfd, (const char *)buffer, MAXLINE, 0, (const struct sockaddr *)&cliaddr, &len);
			if(buffer[0] == 'W'){
				fscanf(f,"%s",words);
				sendto(sockfd, (const char *)words, strlen(words), 0, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
				printf("Sent %s\n", words);
			}
			else{
				flag=1;
				fclose(f);
			}
		}while(!flag);
	}

	close(sockfd);
	return 0;
}