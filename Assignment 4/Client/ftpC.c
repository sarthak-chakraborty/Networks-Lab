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
#define MAX 80
#define PORT_X 50000

void main(){
	int ctrlsockfd, datasockfd, newdatasockfd;
	struct sockaddr_in cli_addr, ctrlserv_addr, dataserv_addr;
	int servlen;
	char mssg[80];
	int code;
	int i, flag;
	long int Y;

	
	if((ctrlsockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket creation failed\n");
		exit(0);
	}
	
	ctrlserv_addr.sin_family = AF_INET;
	ctrlserv_addr.sin_addr.s_addr = INADDR_ANY;
	ctrlserv_addr.sin_port = htons(PORT_X);
	
	if(connect(ctrlsockfd, (struct sockaddr *)&ctrlserv_addr, sizeof(ctrlserv_addr)) < 0){
		perror("Unable to connect to server\n");
		exit(0);
	}
	
	printf("\n> ");
	gets(mssg);
	send(ctrlsockfd, mssg, strlen(mssg)+1, 0);

	recv(ctrlsockfd, &code, 3, 0);
	printf("CODE: %d\n",code);
	
	if(code==200){
		printf("Successful\n");

		if(fork()==0){
			printf("%s\n", mssg);
			i=5;
			while(mssg[i]==' ') i++;
			while(mssg[i] != ' ' && mssg[i] != '\0'){
				Y = 10*Y + (int)(mssg[i] - '0');
				i++;
			}
			printf("Y: %ld\n",Y);
		}
		// 	if((datasockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		// 		perror("Socket creation failed\n");
		// 		exit(0);
		// 	}
			
		// 	cli_addr.sin_family = AF_INET;
		// 	cli_addr.sin_addr.s_addr = INADDR_ANY;
		// 	cli_addr.sin_port = htons(Y);
			
		// 	if(bind(datasockfd,(const struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0){
		// 		perror("Binding error\n");
		// 		exit(0);
		// 	}
		// 	listen(datasockfd, 5);
		// 	printf("\nClient Server Running...\n");

		// 	while(1){
		// 		servlen = sizeof(dataserv_addr);
		// 		newdatasockfd = accept(datasockfd, (struct sockaddr *)&dataserv_addr, &servlen);

		// 		printf("> ");
		// 		gets(mssg);
		// 		printf("MSSG: %s\n", mssg);

		// 		send(newdatasockfd, mssg, strlen(mssg)+1, 0);
		// 	}
			
		// }
	}
	else if(code==503){
		printf("Wrong Command\n");
		close(ctrlsockfd);
		exit(0);
	}
	else if(code==550){
		printf("Wrong Port Number\n");
		close(ctrlsockfd);
		exit(0);
		
	}
}
