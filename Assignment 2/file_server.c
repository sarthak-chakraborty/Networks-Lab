#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#define MAX 100 
#define PACKET 50

void main(){
	int sockfd, newsockfd;
	int clilen;
	struct sockaddr_in cliaddr, servaddr;
	int i, j, fd, flag;
	int rret;
	char c;
	char buff[MAX];
	char file[MAX];

	// Creating the TCP socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket crearion failed.\n");
		exit(0);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(6666);

	// Binding the socket
	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Binding failed\n");
		exit(0);
	}

	// Listen to atmost 2 clients at a time
	listen(sockfd, 2);

	// Server ruuning
	printf("Server Running...\n");
	while(1){
		clilen = sizeof(cliaddr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);	// Accepting the connection of client.

		if (newsockfd < 0){
			perror("Accept error\n");
			exit(0);
		}

		/* Receiving the file name.
		   Server will receive file name uptil there is no null character at the end
		*/
		j = 0;
		flag=0;
		while(rret=recv(newsockfd, buff, PACKET, 0)){
			for(i=0; i<rret; i++){
				file[j++] = buff[i];
				if(buff[i] == '\0'){
					flag=1;
					break;
				}
			}
			if(flag)
				break;
		}
		file[j] = '\0';
		printf("\nFILENAME: %s\n",file);
		fd = open(file, O_RDONLY);

		// If fd is positive, it means the file exists
		if(fd > 0){
			printf("MESSAGE:\n");

			/* Reading the file.
			   EOF is determined by comparing the number of bytes read with the packet size.
			   If the number of bytes read is less, it means the file has ended and so '\0'
			   is appended at the end of the string and send to the client.
			*/
			while(rret=read(fd, buff, PACKET)){
				if(rret < PACKET){
					for(i=rret; i<PACKET; i++)
						buff[i] = '\0';
				}
				for(i=0;i<rret;i++)
					printf("%c",buff[i]);
				send(newsockfd, buff, PACKET, 0);
			}
		}
		close(newsockfd);	// Closing the socket.
	}
	close(sockfd);
}