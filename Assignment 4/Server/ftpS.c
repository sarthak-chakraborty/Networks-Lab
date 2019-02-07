	/*
		FTP SERVER
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

#define MAX 80			// Max length of commands
#define PACKET 50		// Packet length to send data
#define PORT_X 50000	// Port of TCP Control Server

void main(){
	int ctrlsockfd, datasockfd, newctrlsockfd;
	struct sockaddr_in ctrlcli_addr, datacli_addr, serv_addr;
	int clilen;
	char cmnd[MAX];
	char mssg[MAX];
	char buff[100];
	char data[103];
	int i, j, flag;
	long int Y;

	// Codes to send
	int code_OK = htons(200);
	int err1 = htons(501);
	int err2 = htons(503);
	int err3 = htons(550);
	int err4 = htons(502);
	int code_SUCC = htons(250);
	int code_QUIT = htons(421);
	
	// Creating control socket
	if((ctrlsockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket creation failed\n");
		exit(0);
	}

	// Initializing the socket address to 0
	memset(&serv_addr, 0, sizeof(serv_addr));
	memset(&ctrlcli_addr, 0, sizeof(ctrlcli_addr));
	memset(&datacli_addr, 0, sizeof(datacli_addr));

	// Specifying the control server address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_X);
	
	if (setsockopt(ctrlsockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
		perror("setsockopt(SO_REUSEADDR) failed");
		exit(0);
	}

	// Binding the control server
	if(bind(ctrlsockfd,(const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Binding error\n");
		exit(0);
	}
	
	// Server Running
	listen(ctrlsockfd, 5);
	printf("Server Running...\n");


	// The server is always running
	while(1){

		// Loop for the first command which must be of 'port'

		/* First Control Command must be of the port at which the client will open a TCP server.
		   Loop until we get the first command as port
		*/
		while(1){
			clilen = sizeof(ctrlcli_addr);
			// Accept control connection form client
			newctrlsockfd = accept(ctrlsockfd, (struct sockaddr *)&ctrlcli_addr, &clilen);

			// Receive First Control command
			recv(newctrlsockfd, cmnd, 80, 0);
			printf("MSSG: %s\n",cmnd);


			// Handling the leading spaces
			i=0;
			while(cmnd[i]==' ' || cmnd[i]=='\t') i++;
			j=0;
			while(cmnd[i] != '\0'){
				mssg[j++] = cmnd[i];
				i++;
			}
			mssg[j] = '\0';
			

			Y = 0;
			flag = 0;
			// Checking if the command starts with 'port'
			if(mssg[0]=='p' && mssg[1]=='o' && mssg[2]=='r' && mssg[3]=='t' && mssg[4]==' '){
				i=5;

				/* Iterate through all the spaces.
				   Then get the port number and store it in a variable Y.
				   The command must not have any other characters except spaces after the port number, else flag an error.
				*/
				while(mssg[i]==' ' || mssg[i]=='\t') i++;
				while(mssg[i] != ' ' && mssg[i] != '\0'){
					Y = 10*Y + (int)(mssg[i] - '0');
					i++;
				}
				while(mssg[i++] != '\0'){
					if(mssg[i] != ' ' && mssg[i] != '\t'){
						flag = 1;
						break;
					}
				}

				// If error is flagged, send appropriate error code and continue in the loop, else send code = 200
				if(flag == 1){
					send(newctrlsockfd, &err2, sizeof(err2), 0);
					close(newctrlsockfd);
					continue;
				}
				// The value of port must lie between 1024 and 65535
				if(Y>1024 && Y<65535){
					send(newctrlsockfd, &code_OK, sizeof(code_OK), 0);
					break;
				}
				else{
					send(newctrlsockfd, &err3, sizeof(err3), 0);
					close(newctrlsockfd);
					continue;
				}
			}
			else{
				send(newctrlsockfd, &err2, sizeof(err2), 0);
				close(newctrlsockfd);
				continue;
			}
		}



		// LOOP for the rest of the control commands until 'quit'

		/*  This loop check for the command.
		    Performs appropriate tasks like fork() and sends data over data socket
		*/
		while(1){
			fflush(stdin);
			char dir[MAX];
			char file[MAX];
			flag = 0;

			// Receive the control command over control socket
			recv(newctrlsockfd, cmnd, 80, 0);
			printf("%s\n", cmnd);


			// Handling the leading spaces
			i=0;
			while(cmnd[i]==' ' || cmnd[i]=='\t') i++;
			j=0;
			while(cmnd[i] != '\0'){
				mssg[j++] = cmnd[i];
				i++;
			}
			mssg[j] = '\0';


			// 'QUIT' COMMAND
			// If the command is quit, send a code = 421 and quit the process and return to accept from control socket
			if(mssg[0]=='q' && mssg[1]=='u' && mssg[2]=='i' && mssg[3]=='t'){
				i=4;
				while(mssg[i] != '\0'){
					if(mssg[i] != ' ' && mssg[i] != '\t'){
						flag=1;
						break;
					}
					i++;
				}
				if(flag){
					send(newctrlsockfd, &err4, sizeof(err4), 0);
					continue;
				}
				send(newctrlsockfd, &code_QUIT, sizeof(code_OK), 0);
				close(newctrlsockfd);
				break;
			}




			// 'CD' COMMAND
			/* If the command is 'cd', them get the directory name and change directory.
			   If successful, send code = 200, else an error code.
			*/
			j = 0;
			if(mssg[0]=='c' && mssg[1]=='d' && mssg[2]==' '){
				i=3;
				while(mssg[i]==' ' || mssg[i]=='\t') i++;
				while(mssg[i] != ' ' && mssg[i] != '\0'){
					dir[j++] = mssg[i];
					i++;
				}
				dir[j] = '\0';
				while(mssg[i++] != '\0'){
					if(mssg[i] != ' ' && mssg[i] != '\t'){
						flag = 1;
						break;
					}
				}
				if(flag){
					send(newctrlsockfd, &err1, sizeof(err1), 0);
					continue;
				}
				if(chdir(dir) < -1)
					send(newctrlsockfd, &err1, sizeof(err1), 0);
				else
					send(newctrlsockfd, &code_OK, sizeof(code_OK), 0);
			}
		


			// 'GET' COMMAND
			// If the command is 'get', store the filename in a string.
			else if(mssg[0]=='g' && mssg[1]=='e' && mssg[2]=='t' && mssg[3]==' '){
				i=4;
				while(mssg[i]==' ') i++;
				while(mssg[i] != ' ' && mssg[i] != '\0'){
					file[j++] = mssg[i];
					i++;
				}
				file[j] = '\0';
				printf("%s\n",file);
				// File must not start with './' or '../'
				if((file[0]=='.' && file[1]=='/') || file[0]=='.' && file[1]=='.' && file[2]=='/' )
					flag = 1;
				while(mssg[i++] != '\0'){
					if(mssg[i] != ' ' && mssg[i] != '\t'){
						flag = 1;
						break;
					}
				}
				if(flag){
					send(newctrlsockfd, &err1, sizeof(err1), 0);
					continue;
				}

				// Open the file to be read
				int fd = open(file, O_RDONLY);

				// If the file can be read, then fork a child process
				if(fd > 0){

					// Child Process, S_D
					if(fork()==0){
						// Create a TCP client for the exchange of data to the client

						// Create a data socket
						if((datasockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
							perror("Socket creation failed\n");
							exit(0);
						}
						
						// Connect the data socket at port Y
						datacli_addr.sin_family = AF_INET;
						datacli_addr.sin_addr.s_addr = INADDR_ANY;
						datacli_addr.sin_port = htons(Y);

						// Connect to the server at cleint side
						usleep(1000);
						if(connect(datasockfd, (struct sockaddr *)&datacli_addr, sizeof(datacli_addr)) < 0){
							perror("Unable to connect to server\n");
							exit(0);
						}

						// Read data from the file and send it to client
						int rret;
						int err;
						while(rret=read(fd, buff, PACKET)){
							if(rret < PACKET)
								data[0] = 'L';

							else
								data[0] = 'N';
							data[1] = (rret >> 8) & 0xFF;
							data[2] = (rret) & 0xFF;
							for(i=0; i<rret; i++)
								data[i+3] = buff[i];
							if(data[0] == 'L')
								err = send(datasockfd, data, rret+3, 0);
							else
								err = send(datasockfd, data, PACKET+3, 0);

							if(err == -1)
								exit(1);
						}	
						close(fd);
						close(datasockfd);
						exit(0);

					}
					else{
						// Wait if the file is opened for read and check the exit status and send appropriate code
						int status;
						int pid = wait(&status);
						if(WIFEXITED(status)){
							if(WEXITSTATUS(status) == 0)
								send(newctrlsockfd, &code_SUCC, sizeof(code_SUCC), 0);
							else
								send(newctrlsockfd, &err3, sizeof(err3), 0);
						}
					}
				}
				else{
					// Send error code if the file cannot be opened for read
					send(newctrlsockfd, &err3, sizeof(err3), 0);
				}
			}



			// 'PUT' COMMAND
			// If the command is 'put', store the filename in a string.
			else if(mssg[0]=='p' && mssg[1]=='u' && mssg[2]=='t' && mssg[3]==' '){
				i=4;
				while(mssg[i]==' ') i++;
				while(mssg[i] != ' ' && mssg[i] != '\0'){
					file[j++] = mssg[i];
					i++;
				}
				file[j] = '\0';
				printf("%s\n",file);
				while(mssg[i++] != '\0'){
					if(mssg[i] != ' ' && mssg[i] != '\t'){
						flag = 1;
						break;
					}
				}
				// Filename cannot be a path
				for(i=0; i<j; i++){
					if(file[i]=='/')
						flag = 1;
				}
				if(flag){
					send(newctrlsockfd, &err1, sizeof(err1), 0);
					continue;
				}

				pid_t pid;
				if((pid = fork()) < 0){
					perror("Unable to fork a server\n");
					continue;
				}

				// Chils Process, S_D
				if(pid==0){
					// Create a TCP client for the exchange of data to the client

					// Create a data socket
					if((datasockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
						perror("Socket creation failed\n");
						exit(0);
					}
					
					// Connect the data socket at port Y
					datacli_addr.sin_family = AF_INET;
					datacli_addr.sin_addr.s_addr = INADDR_ANY;
					datacli_addr.sin_port = htons(Y);

					// Connect to the server at cleint side
					usleep(1000);
					if(connect(datasockfd, (struct sockaddr *)&datacli_addr, sizeof(datacli_addr)) < 0){
						perror("Unable to connect to server\n");
						exit(0);
					}

					// Write the data in the file by creating it.
					int rret;
					int c = 0;
					int fd;
					char header;
					char character;
					unsigned char bytes[2];
					while(recv(datasockfd, &header, 1, 0)){
						if(c==0)
							fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
						unsigned short int n;
						recv(datasockfd, bytes, 2, 0);
						n = (int)bytes[0];
						n = n << 8;
						n += (int)bytes[1];
						for(i=0; i<n; i++){
							recv(datasockfd, &character, 1, 0);
							write(fd, &character, 1);
						}
						c=1;
						if(header == 'L')
							break;
					}
					close(fd);
					close(datasockfd);
					// Exit with appropriate status
					if(rret == -1)
						exit(1);
					exit(0);

				}
				else{
					// Wait for the child process to complete, check the exit status and send appropriate code
					int status;
					int pid = wait(&status);
					if(WIFEXITED(status)){
						if(WEXITSTATUS(status) == 0)
							send(newctrlsockfd, &code_SUCC, sizeof(code_SUCC), 0);
						else
							send(newctrlsockfd, &err3, sizeof(err3), 0);
					}
				}
				
			}


			// If the command is not among these, then send an error code
			else{
				send(newctrlsockfd, &err4, sizeof(err4), 0);
			}

		}
	}
	close(ctrlsockfd);
}
