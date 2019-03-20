// Include the header file
#include "rsocket.h"

// Global declaration for the various buffers used
recv_buff recv_buff_table;
unack_mssg *unack_mssg_table;
int *recv_mssg_id_table;

int id;
int size_unack = 0;
int count_transmission;
int sockfd;

// Declaration of mutex locks
pthread_mutex_t recv_buff_lock;
pthread_mutex_t unack_lock;



// Function to handle retransmitting of the messages
void handleRetransmit(){
	time_t t;
	time(&t);
	int i;

	// Do for each unacknowledged message
	for(i=0; i<size_unack; i++){

		// If a timeout occurred for the unacknowledged message
		if((t - unack_mssg_table[i].sent_time) >= T_SEC){
			// Sent the unacknowledged message with proper header 
			char num[4];
			sprintf(num, "%03d", unack_mssg_table[i].id);
			char mssg[104];
			memset(mssg, 0, sizeof(mssg));

			strcat(mssg, "M");
			strcat(mssg, num);
			strcat(mssg, unack_mssg_table[i].mssg);
			sendto(sockfd, mssg, strlen(mssg), unack_mssg_table[i].flags, &unack_mssg_table[i].dest_addr, sizeof(unack_mssg_table[i].dest_addr));
			
			count_transmission++; // Increment the total transmission count

			// Update the time when the message is retransmitted
			pthread_mutex_lock(&unack_lock);
			unack_mssg_table[i].sent_time = t;
			pthread_mutex_unlock(&unack_lock);
		}
	}
	return;
}



// Function that receives the messages and takes proper actions
void handleReceive(){
	char buf[104];
	struct sockaddr_in cliaddr;
	socklen_t clilen;

	clilen = sizeof(cliaddr);
	memset(buf, 0, sizeof(buf));
	int n = recvfrom(sockfd, buf, 104, 0, (struct sockaddr *)&cliaddr, &clilen);	// Receive message
	if(n<0){
		perror("Error detected\n");
		return;
	}
	buf[n] = '\0';


	/* To make an unreliable channel, us the function drop probability that
	   return whether a randomly generated number is more than a certain probability.
	   -> If yes, then accept the received message.
	   -> If no, then drop the message.
	*/
	if(dropMessage(PROB) == 0){
		// Check whether the received message is an Application Message or an ACK message and take appropriate steps
		if(buf[0] == 'M')
			handleAppMsgRecv(buf, (struct sockaddr *)&cliaddr, clilen);
		else if(buf[0] == 'A')
			handleACKMsgRecv(buf);
	}
	return;
}



// Function that performs necessary steps when an application message is received
void handleAppMsgRecv(char *buf, struct sockaddr *cliaddr, socklen_t clilen){
	int i;

	// Extract the ID of the message
	char num[4] = {buf[1], buf[2], buf[3], '\0'};
	int id = atoi(num);

	// If the message had already been received, i.e., present in the recv_mssg_id buffer, then just send an ACK and return
	if(recv_mssg_id_table[id-1] == 1){
		char ack[5];
		ack[0] = 'A';
		ack[1] = '\0';
		strcat(ack, num);
		ack[4] = '\0';
		sendto(sockfd, ack, strlen(ack), 0, cliaddr, clilen);

		return;
	}

	// Extract the message present in the received packet
	char mssg[100];
	memset(mssg, 0, strlen(mssg));
	memcpy(mssg, &buf[4], 100);

	// Update the recv_buff with the message
	pthread_mutex_lock(&recv_buff_lock);

	int rear = recv_buff_table.rear;
	strcpy(recv_buff_table.mssg[rear].buff, mssg);
	recv_buff_table.mssg[rear].src_addr = *cliaddr;
	recv_buff_table.rear++;

	pthread_mutex_unlock(&recv_buff_lock);

	// Send ACK for the received message
	char ack[5];
	ack[0] = 'A';
	ack[1] = '\0';
	strcat(ack, num);
	ack[4] = '\0';
	sendto(sockfd, ack, strlen(ack), 0, cliaddr, clilen);
	
	recv_mssg_id_table[id-1] = 1;	// Update the received_message_id table

	return;
}



// Function that takes necessary steps when ACK message is received
void handleACKMsgRecv(char *buf){

	// Extract the ID of the ACK message
	char num[4] = {buf[1], buf[2], buf[3], '\0'};
	int id = atoi(num);

	// Search for the ID in Unacked mssg table, and remove the entry
	for(int i=0; i<size_unack; i++){
		if(unack_mssg_table[i].id == id){

			pthread_mutex_lock(&unack_lock);
			for(int j=i; j<size_unack - 1; j++){
				unack_mssg_table[j] = unack_mssg_table[j+1];
			}
			size_unack--;
			pthread_mutex_unlock(&unack_lock);
		}
	}
	return;
}



// Runner program for thread X
void *runner(void *parameters){
	fd_set readfs;
	struct timeval t;

	// Thread continuously runs
	while(1){
		FD_ZERO(&readfs);
		FD_SET(sockfd, &readfs);

		if(t.tv_sec == 0){
			t.tv_sec = T_SEC;
			t.tv_usec = T_USEC;
		}
		int r = select((sockfd+1), &readfs, 0, 0, &t);	// Chekc whether the message is received or not within a certain time

		if(r<0)
			continue;
		if(FD_ISSET(sockfd, &readfs))
			handleReceive();
		else
			handleRetransmit();
	}
}



// r_socket function which initializes the socket with an underlying UDP protocol and allocates memory for the various buffers
int r_socket(int domain, int type, int protocol){
	if(type == SOCK_MRP){
		sockfd = socket(domain, SOCK_DGRAM, protocol);

		srand(time(0));

		// Initialize mutex locks
		pthread_mutex_init(&recv_buff_lock, NULL);
		pthread_mutex_init(&unack_lock, NULL);

		// Create thread X
		pthread_t tid;
		pthread_create(&tid, NULL, runner, (void *)NULL);

		// Initialize other global variables and allocate memory to different buffers
		id = 0;
		count_transmission = 0;
		recv_buff_table.mssg = (recv_mssg *)malloc(MAX_SIZE*sizeof(recv_mssg));
		recv_buff_table.front = 0;
		recv_buff_table.rear = 0;
		unack_mssg_table = (unack_mssg *)malloc(MAX_SIZE*sizeof(unack_mssg));
		recv_mssg_id_table = (int *)malloc(MAX_SIZE*sizeof(int));

		for(int i=0; i<MAX_SIZE; i++)
			recv_mssg_id_table[i] = 0;

		return sockfd;
	}
	return -1;
}



// Binds to the socket (underlying UDP socket)
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
	return bind(sockfd, addr, addrlen);
}



// Function to send the message using proper headers
int r_sendto(int sockfd, const void *buff, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen){

	// Convert the integer ID to string
	char num[4];
	sprintf(num, "%03d", id);
	char mssg[104];
	memset(mssg, 0, sizeof(mssg));

	char buf[100];
	memset(buf, 0, sizeof(buf));
	memcpy(buf, (&buff)[0], len);

	// Append the header to the message
	strcat(mssg, "M");
	strcat(mssg, num);
	strcat(mssg, buf);

	int n = sendto(sockfd, mssg, len+4, flags, dest_addr, addrlen);	// Send the message
	// If the header was not sent, then return with -1
	if(n<4)
		return -1;
	
	count_transmission++; 	// Imcrement the total transmission count


	// Update the Unacked message table with the message, ID, flags, and destination address
	pthread_mutex_lock(&unack_lock);

	time(&(unack_mssg_table[size_unack].sent_time));
	unack_mssg_table[size_unack].dest_addr = *dest_addr;
	unack_mssg_table[size_unack].id = id;
	strcpy(unack_mssg_table[size_unack].mssg, buf);
	unack_mssg_table[size_unack].flags = flags;
	size_unack++;

	pthread_mutex_unlock(&unack_lock);

	id++;

	return 0;
}



// Fucntion that retrieves a message from the receive buffer and returns to the calling function
int r_recvfrom(int sockfd, void *buff, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
	char *mssg = (char *)buff;

	// If there is nothing in the receive buffer, then wait until there is an entry (busy waiting). Implements blocking call
	while(recv_buff_table.front == recv_buff_table.rear) usleep(5000);

	int front = recv_buff_table.front;

	// If requested length is less than the string length present in the buffer
	if(len < strlen(recv_buff_table.mssg[front].buff)){
		// Copy the message in the recv_buff and store the source address and the address length
		pthread_mutex_lock(&recv_buff_lock);

		for(int i=0; i<len; i++)
			mssg[i] = recv_buff_table.mssg[front].buff[i];
		*src_addr = recv_buff_table.mssg[front].src_addr;
		*addrlen = sizeof(*src_addr);
		recv_buff_table.front++;

		pthread_mutex_unlock(&recv_buff_lock);

		return (int)len;	// Return the length of the string
	}
	else{
		// Copy the message in the buffer and store the soruce address and the length of the address
		pthread_mutex_lock(&recv_buff_lock);

		strcpy(mssg, recv_buff_table.mssg[front].buff);
		*src_addr = recv_buff_table.mssg[front].src_addr;
		*addrlen = sizeof(*src_addr);
		recv_buff_table.front++;

		pthread_mutex_unlock(&recv_buff_lock);

		return (int)strlen(recv_buff_table.mssg[front].buff);	// Return the length of the string (bytes received)
	}
	return -1;
}



// Function that closes the socket connection
int r_close(int sockfd){
	// If there is any message that is still unacknowledged, wait till it gets acknowledged
	while(size_unack != 0){
		usleep(5000);
	}

	// Free the buffers
	free(recv_buff_table.mssg);
	free(unack_mssg_table);
	free(recv_mssg_id_table);

	int n = close(sockfd);
	if(n==0)
		return count_transmission;	// Return the number of transmission that were made to transfer the string

	return -1;
}



// Returns 1 if a generated random number is less than some fixed probability
int dropMessage(float p){
	float n = (float)rand() / RAND_MAX;

	if(n<p)
		return 1;

	return 0;
}
