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
	char buff[MAX];
	int i, j, flag, OK;
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
				send(newctrlsockfd, &err2, sizeof(int), 0);
				close(newctrlsockfd);
				continue;
			}
			if(Y>1024 && Y<65535){
				send(newctrlsockfd, &code_OK, sizeof(int), 0);
				OK = 1;
				break;
			}
			else{
				send(newctrlsockfd, &err3, sizeof(int), 0);
				close(newctrlsockfd);
				continue;
			}
		}
		else{
			send(newctrlsockfd, &err2, sizeof(int), 0);
			close(newctrlsockfd);
			continue;
		}
	}

	while(1){
		fflush(stdin);
		char dir[MAX];
		char file[MAX];
		recv(newctrlsockfd, mssg, 80, 0);
		printf("%s\n", mssg);

		if(!strcmp(mssg, "quit")){
			send(newctrlsockfd, &code_QUIT, sizeof(int), 0);
			close(newctrlsockfd);
			break;
		}
		j = 0;
		if(mssg[0]=='c' && mssg[1]=='d' && mssg[2]==' '){
			i=3;
			while(mssg[i]==' ') i++;
			while(mssg[i] != ' ' && mssg[i] != '\0'){
				dir[j++] = mssg[i];
				i++;
			}
			dir[j] = '\0';
			printf("%s\n", dir);
			if(chdir(dir) < -1)
				send(newctrlsockfd, &err1, sizeof(int), 0);
			else
				send(newctrlsockfd, &code_OK, sizeof(int), 0);
		}
	
		else if(mssg[0]=='g' && mssg[1]=='e' && mssg[2]=='t' && mssg[3]==' '){
			i=4;
			while(mssg[i]==' ') i++;
			while(mssg[i] != ' ' && mssg[i] != '\0'){
				file[j++] = mssg[i];
				i++;
			}
			file[j] = '\0';
			FILE *f = fopen(file, "r");
			if(f > 0){
				if(fork()==0){
					if((datasockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
						perror("Socket creation failed\n");
						exit(0);
					}
					
					datacli_addr.sin_family = AF_INET;
					datacli_addr.sin_addr.s_addr = INADDR_ANY;
					datacli_addr.sin_port = htons(Y);

					if(connect(datasockfd, (struct sockaddr *)&datacli_addr, sizeof(datacli_addr)) < 0){
						perror("Unable to connect to server\n");
						exit(0);
					}
					printf("Connected\n");
					send(datasockfd, "HELLO", 10, 0);
					close(datasockfd);
					exit(0);

				}
				else{
					wait(NULL);
					send(newctrlsockfd, &code_OK, sizeof(int), 0);
				}
			}
			else
				send(newctrlsockfd, &err3, sizeof(int), 0);
		}

	}
		// while(1){
		// 	recv(newctrlsockfd, mssg, 80, 0);
		// 	printf("MSSG Received: %s\n",mssg);


		// }
		// if((datasockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		// 	perror("Socket creation failed\n");
		// 	exit(0);
		// }
		
		// datacli_addr.sin_family = AF_INET;
		// datacli_addr.sin_addr.s_addr = INADDR_ANY;
		// datacli_addr.sin_port = htons(Y);

		// usleep(2000);
		// if(connect(datasockfd, (struct sockaddr *)&datacli_addr, sizeof(datacli_addr)) < 0){
		// 	perror("Unable to connect to server\n");
		// 	exit(0);
		// }

		// printf("Connected\n");

		// while(1){
		// 	recv(datasockfd, buff, 80, 0);
		// 	printf("MSSG Received: %s\n", buff);
		// }
	
}
