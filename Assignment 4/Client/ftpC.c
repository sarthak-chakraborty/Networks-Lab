	/*
		FTP CLIENT
	*/


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
#include <unistd.h>
#include <signal.h>

#define MAX 80			// Max length of the control commands
#define PACKET 20		// Packet length for data transfer
#define PORT_X 50000	// Port at which server runs a TCP server

void main(){
	int ctrlsockfd, datasockfd, newdatasockfd;
	struct sockaddr_in cli_addr, ctrlserv_addr, dataserv_addr;
	int servlen;
	char mssg[MAX];
	char buff[100];
	char data[100];
	char file[MAX];
	int code;
	int i, j, flag;
	long int Y;

	// Creating the control socket
	if((ctrlsockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket creation failed\n");
		exit(0);
	}

	// Initializing the socket addresses to 0
	memset(&ctrlserv_addr, 0, sizeof(ctrlserv_addr));
	memset(&dataserv_addr, 0, sizeof(dataserv_addr));
	memset(&cli_addr, 0, sizeof(cli_addr));
	
	// Specifying the address of the control server at server
	ctrlserv_addr.sin_family = AF_INET;
	ctrlserv_addr.sin_addr.s_addr = INADDR_ANY;
	ctrlserv_addr.sin_port = htons(PORT_X);
	
	// Connecting to the control server
	if(connect(ctrlsockfd, (struct sockaddr *)&ctrlserv_addr, sizeof(ctrlserv_addr)) < 0){
		perror("Unable to connect to server\n");
		exit(0);
	}
	

	// Prompt for the first command
	printf("> ");
	scanf("%[^\n]s",mssg);
	send(ctrlsockfd, mssg, strlen(mssg)+1, 0);
	recv(ctrlsockfd, &code, sizeof(code), 0);
	printf("CODE: %d\n", ntohl(code));

	// If the received coe is 200, that is successful, parse the port number from the string
	if(ntohl(code)==200){
		printf("MSSG: Valid Port number\n");
		Y = 0;
		i=5;
		while(mssg[i]==' ') i++;
		while(mssg[i] != ' ' && mssg[i] != '\0'){
			Y = 10*Y + (int)(mssg[i] - '0');
			i++;
		}
	}
	// If any other error code is received, print appropriate message and exit
	else if(ntohl(code)==503){
		printf("MSSG: Invalid Command\n");
		close(ctrlsockfd);
		exit(0);
	}
	else if(ntohl(code)==550){
		printf("MSSG: Invalid Port Number\n");
		close(ctrlsockfd);
		exit(0);
		
	}




	/* Now, take the command as user input in a loop.
	   Send it to the server using control socket and perform necessary actions.
	*/
	while(1){
		fflush(stdin);
		getchar();
		printf("\n> ");
		scanf("%[^\n]s", mssg);



		/* If the message is a 'get' message, fork a child process
		   which will be the TCP server opened in the client. Receive
		   the file from the server and then kill the child process, 
		   that is the server
		*/
		if(mssg[0]=='g' && mssg[1]=='e' && mssg[2]=='t' && mssg[3]==' '){
			pid_t pid;

			// Fork a child process
			if((pid = fork()) < 0){
				perror("Unable to fork a server\n");
				continue;
			}

			// Child Process, C_D
			if(pid==0){
				i=4;
				j=0;

				// Parse for the file name
				while(mssg[i]==' ') i++;
				while(mssg[i] != ' ' && mssg[i] != '\0'){
					file[j++] = mssg[i];
					i++;
				}
				file[j] = '\0';

				char *word;
				char *file1;
				word = strtok(file,"/");
				while(word!=NULL){
					file1 = word;
					word = strtok(NULL,"/");
				}
				int fd;


				// Create data socket
				if((datasockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
					perror("Socket creation failed\n");
					exit(0);
				}
				
				// Specify the address for the client TCP server
				cli_addr.sin_family = AF_INET;
				cli_addr.sin_addr.s_addr = INADDR_ANY;
				cli_addr.sin_port = htons(Y);
				
				if (setsockopt(datasockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
    				perror("setsockopt(SO_REUSEADDR) failed");
    				exit(0);
				}

				// Bind to the address to the socket
				if(bind(datasockfd,(const struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0){
					perror("Binding error\n");
					exit(0);
				}
				listen(datasockfd, 5);

				// Accept the connection from the TCP client at server side
				servlen = sizeof(dataserv_addr);
				newdatasockfd = accept(datasockfd, (struct sockaddr *)&dataserv_addr, &servlen);

				// Receive the data from the data socket created and write it to a file in client side
				int rret;
				int c = 0;
				while(rret=recv(newdatasockfd, buff, PACKET, 0)){
					if(c==0)
						fd = open(file1, O_WRONLY|O_CREAT|O_TRUNC);
					for(i=0; i<rret; i++){
						if(buff[i] != '\0')
							write(fd, &buff[i], 1);
					}
					c=1;
				}
				close(fd);
				close(newdatasockfd);
				exit(0);
			}	
			else{
				send(ctrlsockfd, mssg, strlen(mssg) + 1, 0);
				// Receive the response code from the server
				recv(ctrlsockfd, &code, sizeof(code), 0);
				printf("CODE: %d\n", ntohl(code));

				// If error code, then kill the process C_D
				if(ntohl(code) == 550){
					printf("MSSG: An error occured while receiving the file\n");
					kill(pid, SIGTERM);
				}
				// If the error code is 250, then wait for the process to complete
				else if(ntohl(code) == 250){
					wait(NULL);
					printf("MSSG: File successfully received\n");
				}
			}
		}



		/* If the message is a 'put' message, fork a child process
		   which will be the TCP server opened in the client. Send
		   the file to the server and close the connection if successful.
		*/
		else if(mssg[0]=='p' && mssg[1]=='u' && mssg[2]=='t' && mssg[3]==' '){

			// Parse the file name from the string
			i=4;
			j=0;
			while(mssg[i]==' ') i++;
			while(mssg[i] != ' ' && mssg[i] != '\0'){
				file[j++] = mssg[i];
				i++;
			}
			file[j] = '\0';

			int fd = open(file, O_RDONLY);
			if(fd < 0){
				printf("CODE: 550\n");
				printf("MSSG: Error occured while opening the file\n");
				continue;
			}

			int pid = fork();
			// Child Process, that is, C_D
			if(pid==0){

				// Create a socket
				if((datasockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
					perror("Socket creation failed\n");
					exit(0);
				}
				// Specify the address of the socket
				cli_addr.sin_family = AF_INET;
				cli_addr.sin_addr.s_addr = INADDR_ANY;
				cli_addr.sin_port = htons(Y);
				
				if (setsockopt(datasockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
    				perror("setsockopt(SO_REUSEADDR) failed");
    				exit(0);
				}

				// Bind the socket
				if(bind(datasockfd,(const struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0){
					perror("Binding error\n");
					exit(0);
				}
				listen(datasockfd, 5);

				// Accept connection to the TCPserver
				servlen = sizeof(dataserv_addr);
				newdatasockfd = accept(datasockfd, (struct sockaddr *)&dataserv_addr, &servlen);

				// Send the contents of the file to the server
				int rret;
				while(rret=read(fd, buff, PACKET)){
					if(rret < PACKET){
						data[0] = 'N';
						for(i=rret; i<PACKET; i++)
							buff[i] = '\0';
					}
					
					send(newdatasockfd, buff, PACKET, 0);
				}
				close(fd);
				wait(NULL);
			}
			
			else{
				send(ctrlsockfd, mssg, strlen(mssg) + 1, 0);
				// Receive the code from the server
				recv(ctrlsockfd, &code, sizeof(code), 0);
				printf("CODE: %d\n", ntohl(code));

				// If error code, kill the process
				if(ntohl(code) == 550){
					printf("MSSG: An error occurred while sending the file\n");
					kill(pid, SIGTERM);
				}
				// If successful, wait for the child to close.
				else if(ntohl(code) == 250){
					kill(pid, SIGTERM);
					printf("MSSG: File successfully sent\n");
				}

			}
		}
		

		// If not a 'get' or 'put' command, send it to the server
		else{		
			send(ctrlsockfd, mssg, strlen(mssg) + 1, 0);
			recv(ctrlsockfd, &code, sizeof(code), 0);
			printf("CODE: %d\n", ntohl(code));

			// If code = 421, then exit from the client
			if(ntohl(code)==421){
				printf("MSSG: Exit client\n");
				close(ctrlsockfd);
				exit(0);
			}
			else if(ntohl(code)==502)
				printf("MSSG: Invalid Command\n");
			else if(ntohl(code)==200)
				printf("MSSG: Successfully executed the command\n");
			else if(ntohl(code)==501)
				printf("MSSG: Invalid format of the command\n");
		}
	}
}
