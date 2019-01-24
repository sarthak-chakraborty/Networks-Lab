#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h> 
#define MAX 100
#define PACKET 10

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
	char c, prev = ' ';
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
	printf("Filename sent\n");

	/* Run the loop until something is being received from the server.
	   With every character that is received, increase byte count by 1.
	   Check the condition for it being a word. If a null character is received,
	   it means that there is a EOF fron the server side, hence, break from the loop.
	*/
	while(rret=recv(sockfd, buff, PACKET, 0)){
		if(!b_count)
			fd = open("clientfile.txt", O_WRONLY|O_CREAT|O_TRUNC);
		for(i=0; i<rret; i++){
			if(buff[i] != '\0'){
				write(fd, &buff[i], 1);
				b_count++;
				if(!search(buff[i], delim) && search(prev, delim))
					w_count++;
				prev = buff[i];
			}
		}
	}


	/* We had to assume that if there is a file in the server directory,
	   then its size must be greater than 0 bytes. So if bytes count is
	   0, it means the file is not found by the server.
	*/
	if(b_count == 0){
		printf("File Not Found.\n");
		exit(0);
	}
	
	printf("Bytes Count: %d\n", b_count);
	printf("Words Count: %d\n", w_count);

	close(sockfd);

}