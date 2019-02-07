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

// Function to search if the character is one of the delimiters
int search(char c, char arr[]){
	int n = sizeof(arr)/sizeof(char);
	for(int i=0; i<n; i++){
		if(c==arr[i])
			return 1;
	}
	return 0;
}

void main(){
	int sockfd;
	int clilen;
	struct sockaddr_in servaddr;
	char file[MAX];
	char buff[MAX];
	int c;
	char delim[] = {',' , ';' , ':' , '.' , ' ' , '\t' , '\n'};		// Delimiters
	int i, rret, fd, b_count = 0, w_count = 0;

	// Creating the socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket crearion failed.\n");
		exit(0);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(6666);

	// Connecting the socket with the server
	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Unable to connect to server\n");
		exit(0);
	}

	// Sending the file name to the server
	printf("Enter file name: ");
	scanf("%s",file);
	send(sockfd, file, strlen(file)+1, 0);

	// Receive the character at first
	char a;
	recv(sockfd, &a, 1, 0);

	// If the character is 'E', then file is not found and close the socket
	if(a=='E'){
		printf("File not found\n");
		close(sockfd);
		exit(0);
	}
	/* If the character is 'L', then get the size of the file.
	   Now for the fixed block size, receive the contents of the file.
	   Also, the last block will not have the specified size, so
	   handle that accordingly
	*/
	else if(a=='L'){
		int size;
		recv(sockfd, &size, 2, MSG_WAITALL);
		size = ntohs(size);

		// Recv the file contents
		c = 0;
		for(i=0; i<(size/BLOCK); i++){
			if(c == 0)
				fd = open("clientfile.txt", O_WRONLY|O_CREAT|O_TRUNC, 0666);
			recv(sockfd, buff, BLOCK, MSG_WAITALL);
			write(fd, buff, BLOCK);
			c = 1;
		}
		recv(sockfd, buff, (size%BLOCK), MSG_WAITALL);
		write(fd, buff, (size%BLOCK));

		close(fd);
		
		printf("\nThe file transfer is successful.\n");
		printf("Total number of blocks received = %d\n",i);
		printf("Last block size = %d\n", (size%BLOCK));
	}
	
	close(sockfd);

}