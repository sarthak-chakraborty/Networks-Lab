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
	int sockfd, flag = 0, count = 1;
	struct sockaddr_in servaddr;
	FILE *f;

	// Creating the socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sockfd < 0){
		perror("Socket Creation Failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(6666);

	int n;
	socklen_t len;
	char file[MAXLINE];
	printf("Enter file name: ");	// Filename to be entered
	scanf("%s",file);

	sendto(sockfd, (const char *)file, strlen(file), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

	do{		// Get the required word from the server until "END" is received
		char mssg[MAXLINE];
		char word[MAXLINE] = "WORD";
		char num[3];  
    	sprintf(num, "%d", count);
		strcat(word, num);	// Concatinating "WORD" with the number
		n = recvfrom(sockfd, (const char *)mssg, MAXLINE, 0, (const struct sockaddr *)&servaddr, &len);	// Get the required word from server
		mssg[n] = '\0';

		if(!strcmp(mssg, "NOTFOUND")){
			printf("File Not Found\n");		// If not found, exit
			exit(EXIT_FAILURE);
		}
		else if(!strcmp(mssg, "HELLO")){	// If "HELLO", then create a text file and request for the first word after "HELLO"
			printf("Check \'clientfile.txt\' for output.\n");
			f = fopen("clientfile.txt","w");
			sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
			count++;
		}
		else if(!strcmp(mssg, "END")){	// If "END", then close the file
			flag = 1;
			char *end = "END";
			sendto(sockfd, (const char *)end, strlen(end), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
			fclose(f);
		}
		else{	// Write the word in the file
			sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
			fprintf(f,mssg);
			fprintf(f,"\n");
			count++;
		}
	}while(!flag);
	

	close(sockfd);
	return 0;
}