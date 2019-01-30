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
	int ctrlsockfd, datasockfd, newctrlsockfd;
	struct sockaddr_in ctrlcli_addr, datacli_addr, serv_addr;
	int clilen;
	char mssg[MAX];
	int i, flag, OK;
	long int Y;
	int code_OK = 200;
	int err1 = 501;
	int err2 = 503;
	int err3 = 550;
	int code_SUCC = 250;
	int code_QUIT = 421;
	
	if((ctrlsockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket creation failed\n");
		exit(0);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	memset(&ctrlcli_addr, 0, sizeof(ctrlcli_addr));
	memset(&datacli_addr, 0, sizeof(datacli_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_X);
	
	if(bind(ctrlsockfd,(const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Binding error\n");
		exit(0);
	}
	
	listen(ctrlsockfd, 5);
	printf("Server Running...\n");

	while(1){
		clilen = sizeof(ctrlcli_addr);
		newctrlsockfd = accept(ctrlsockfd, (struct sockaddr *)&ctrlcli_addr, &clilen);

		recv(newctrlsockfd, mssg, 80, 0);
		printf("MSSG: %s\n",mssg);
		
		Y = 0;
		flag = 0;
		OK = 0;
		if(mssg[0]=='p' && mssg[1]=='o' && mssg[2]=='r' && mssg[3]=='t' && mssg[4]==' '){
			i=5;
			while(mssg[i]==' ') i++;
			while(mssg[i] != ' ' && mssg[i] != '\0'){
				Y = 10*Y + (int)(mssg[i] - '0');
				i++;
			}
			while(mssg[i++] != '\0'){
				if(mssg[i] != ' '){
					flag = 1;
					break;
				}
			}
			if(flag == 1){
				send(newctrlsockfd, &err2, 3, 0);
				close(newctrlsockfd);
				continue;
			}
			printf("Y: %ld\n",Y);
			if(Y>1024 && Y<65535){
				send(newctrlsockfd, &code_OK, 3, 0);
				OK = 1;
				break;
			}
			else{
				send(newctrlsockfd, &err3, 3, 0);
				close(newctrlsockfd);
				continue;
			}
		}
		else{
			send(newctrlsockfd, &err2, 3, 0);
			close(newctrlsockfd);
			continue;
		}
	}

	if(OK){
		if((datasockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror("Socket creation failed\n");
			exit(0);
		}
		
		datacli_addr.sin_family = AF_INET;
		datacli_addr.sin_addr.s_addr = INADDR_ANY;
		datacli_addr.sin_port = htons(Y);

		usleep(2000);
		if(connect(datasockfd, (struct sockaddr *)&datacli_addr, sizeof(datacli_addr)) < 0){
			perror("Unable to connect to server\n");
			exit(0);
		}

		printf("Connected\n");

		while(1){
			recv(datasockfd, mssg, 80, 0);
			printf("MSSG Received: %s\n", mssg);
		}
	}
}
