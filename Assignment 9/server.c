/* 
Signal driven I/O for UDP socket is simple and may be preferred over non-blocking I/O, since SIGIO is generated only when a datagram has arrived. However this is not the case for TCP sockets. SIGIO is generated not only when an input arrives, but also when a disconnect request is initiated or an ACK message has arrived. Hence the number of such SIGIO generated will be very high. Also, we need to know what type of event has caused the signal. Thus, non-blocking I/O is preferred over signal driven I/O for TCP sockets.

When an interrupt is generated, athe main program gets stalled and the signal handler is invoked. If the same data structure is used in the signal handler as that in the main program, then a race condition may occur. Even if we block the critical section using a semaphore, and an interrupt occurs between the critical section, the signal handler will try to lock the semaphore and hence there will be a deadlock. Thus signal handler must be used carefully.

If the body of the signal handler becomes long, and a signal arrives in between the execution of the signal handler, the signal may get dropped. Hence we need to handle such a case and this makes the use of signal driven I/O difficult.
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define MAXSIZE 100

int sockfd;

void sig_handler(int signo){
	struct sockaddr_in cliaddr;
	int clilen = sizeof(cliaddr);
	char buff[MAXSIZE];
	
	// Receiving from client
	int n = recvfrom(sockfd, buff, MAXSIZE, 0, (struct sockaddr *)&cliaddr, &clilen);
	printf("MESSAGE RECEIVED FROM CLIENT:\n");
	printf("%s\n",buff);

	// Message sent to client
	sendto(sockfd, buff, MAXSIZE, 0, (struct sockaddr *)&cliaddr, clilen);
	printf("MESSAGE SENT TO CLIENT\n\n");
	
	// Re-register signal handler
	signal(SIGIO, sig_handler);
	
	return;
}


int main(){
	signal(SIGIO, sig_handler);
	
	struct sockaddr_in servaddr;
	
	// Socket creation
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("Socket creation failed\n");
		exit(1);
	}
	
	// Initialize servaddr
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(6666);
	
	// Bind to the server
	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Bind failed\n");
		exit(1);
	}
	
	// Set Owner of the socket for the process fro signal driven I/O
	if (fcntl(sockfd, F_SETOWN, getpid()) < 0){
		perror("fcntl F_SETOWN error\n");
		exit(1);
	}
	
	// Set asynchronous flag
	int flags = fcntl(sockfd, F_GETFL);
	if(fcntl(sockfd, F_SETFL, flags|O_ASYNC) < 0){
		perror("fcntl F_SETFL error\n");
		exit(1);
	}
	
	printf("Server Running...\n");
	
	while(1) ;
	
	close(sockfd);
	return 0;
}
