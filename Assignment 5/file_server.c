#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>

#define MAX 100 
#define BLOCK 20

void main(){
	int sockfd, newsockfd;
	int clilen;
	struct sockaddr_in cliaddr, servaddr;
	int i, j, fd, flag;
	short int FSIZE;
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

	// Avoid address in use error
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
		perror("setsockopt(SO_REUSEADDR) failed");
		exit(0);
	}

	// Binding the socket
	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Binding failed\n");
		exit(0);
	}

	// Listen to atmost 5 clients at a time
	listen(sockfd, 5);

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
		while(rret=recv(newsockfd, buff, BLOCK, 0)){
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

		// If the file cannot be opened
		if(fd < 0){
			//Send character 'E' and close the connection

			char a = 'E';
			send(newsockfd, &a, 1, 0);
			close(newsockfd);
			continue;
		}
		else{
			/* If the file can be opened, send a letter 'L' followed by the size of the file
			   to the client. Now, send the file block by block having a specific length in
			   bytes. The last block may have less number of bytes, so send the appropriate
			   number of bytes to the client.
			*/
			char a='L';
			send(newsockfd, &a, 1, 0);

			// Get the file size and send it to client
			FSIZE = lseek(fd, 0, SEEK_END);
			printf("Filesize: %d bytes\n", FSIZE);
			FSIZE = htons(FSIZE);
			send(newsockfd, &FSIZE, 2, 0);

			// Bring the file pointer to the beginning of the file
			lseek(fd, 0, SEEK_SET);

			// Find the number of blocks to be sent
			int blocks = ntohs(FSIZE)/BLOCK;
			int last = ntohs(FSIZE)%BLOCK;

			// Send the contents of the file
			printf("FILE CONTENTS: \n");
			for(i=0; i<blocks; i++){
				read(fd, buff, BLOCK);
				printf("%s", buff);
				send(newsockfd, buff, BLOCK, 0);
			}
			read(fd, buff, last);
			printf("%s\n",buff);
			send(newsockfd, buff, last, 0);

			printf("\nFile sent\n");
			close(fd);
			
		}
		close(newsockfd);
	}

	close(sockfd);
}