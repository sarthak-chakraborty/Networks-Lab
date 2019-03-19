#include "rsocket.h"

// recv_buff *recv_buff_table;
recv_buff recv_buff_table;
unack_mssg *unack_mssg_table;
recv_mssg_id *recv_mssg_id_table;

int id = 1;
int size_unack = 0;
int size_recv_id = 0;
int size_recv_buf = 0;
int count = 1;

int sockfd;

pthread_mutex_t recv_buff_lock;
pthread_mutex_t unack_lock;


void handleRetransmit(){
	time_t t;
	time(&t);
	int i;
	for(i=0; i<size_unack; i++){
		if((t - unack_mssg_table[i].sent_time) >= T_SEC){
			char num[4];
			sprintf(num, "%03d", unack_mssg_table[i].id);
			char mssg[104];
			memset(mssg, NULL, sizeof(mssg));

			strcat(mssg, "M");
			strcat(mssg, num);
			strcat(mssg, unack_mssg_table[i].mssg);
			sendto(sockfd, mssg, strlen(mssg), unack_mssg_table[i].flags, &unack_mssg_table[i].dest_addr, sizeof(unack_mssg_table[i].dest_addr));
		}
	}
	return;
}


void handleReceive(){
	char buf[104];
	struct sockaddr_in cliaddr;
	socklen_t clilen;

	clilen = sizeof(cliaddr);
	memset(buf, NULL, sizeof(buf));
	int n = recvfrom(sockfd, buf, 104, 0, (struct sockaddr *)&cliaddr, &clilen);
	if(n<0){
		perror("Error detected\n");
		return;
	}
	buf[n] = '\0';
	// printf("%d, %s\n",n, buf);

	if(dropMessage(PROB) == 0){
		if(buf[0] == 'M')
			handleAppMsgRecv(buf, (struct sockaddr *)&cliaddr, clilen);
		else if(buf[0] == 'A')
			handleACKMsgRecv(buf);
	}
	return;
}


void handleAppMsgRecv(char *buf, struct sockaddr *cliaddr, socklen_t clilen){
	int i;

	// printf("<%s>\n",buf);

	char num[4] = {buf[1], buf[2], buf[3], '\0'};
	int id = atoi(num);
	for(i=0; i<size_recv_id; i++){
		if(recv_mssg_id_table[i].id == id){
			char ack[5];
			ack[0] = 'A';
			ack[1] = '\0';
			strcat(ack, num);
			ack[4] = '\0';
			sendto(sockfd, ack, strlen(ack), 0, cliaddr, clilen);

			return;
		}
	}
	char mssg[100];
	memset(mssg, NULL, strlen(mssg));
	memcpy(mssg, &buf[4], 100);

	pthread_mutex_lock(&recv_buff_lock);

	// strcpy(recv_buff_table[size_recv_buf].buff, mssg);
	// recv_buff_table[size_recv_buf].src_addr = *cliaddr;
	// size_recv_buf++;
	int rear = recv_buff_table.rear;
	strcpy(recv_buff_table.mssg[rear].buff, mssg);
	recv_buff_table.mssg[rear].src_addr = *cliaddr;
	recv_buff_table.rear++;

	pthread_mutex_unlock(&recv_buff_lock);

	char ack[5];
	ack[0] = 'A';
	ack[1] = '\0';
	strcat(ack, num);
	ack[4] = '\0';
	sendto(sockfd, ack, strlen(ack), 0, cliaddr, clilen);
	
	recv_mssg_id_table[size_recv_id].id = id;
	size_recv_id++;

	return;
}


void handleACKMsgRecv(char *buf){

	char num[4] = {buf[1], buf[2], buf[3], '\0'};
	int id = atoi(num);

	// printf("<%s, %d>\n",buf, id);

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


void *runner(void *parameters){
	fd_set readfs;
	struct timeval t;

	while(1){
		FD_ZERO(&readfs);
		FD_SET(sockfd, &readfs);

		if(t.tv_sec == 0){
			t.tv_sec = T_SEC;
			t.tv_usec = T_USEC;
		}
		int r = select((sockfd+1), &readfs, 0, 0, &t);

		if(r<0)
			continue;
		if(FD_ISSET(sockfd, &readfs))
			handleReceive();
		else
			handleRetransmit();
	}
}



int r_socket(int domain, int type, int protocol){
	if(type == SOCK_MRP){
		sockfd = socket(domain, SOCK_DGRAM, protocol);

		srand(time(0));

		pthread_mutex_init(&recv_buff_lock, NULL);
		pthread_mutex_init(&unack_lock, NULL);

		pthread_t tid;
		pthread_create(&tid, NULL, runner, (void *)NULL);

		// recv_buff_table = (recv_buff *)malloc(MAX_SIZE*sizeof(recv_buff));
		recv_buff_table.mssg = (recv_mssg *)malloc(MAX_SIZE*sizeof(recv_mssg));
		recv_buff_table.front = 0;
		recv_buff_table.rear = 0;
		unack_mssg_table = (unack_mssg *)malloc(MAX_SIZE*sizeof(unack_mssg));
		recv_mssg_id_table = (recv_mssg_id *)malloc(MAX_SIZE*sizeof(recv_mssg_id));

		return sockfd;
	}
	return -1;
}


int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
	int n = bind(sockfd, addr, addrlen);
	if(n>0)
		return 0;

	return -1;
}


int r_sendto(int sockfd, const void *buff, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen){
	char num[4];
	sprintf(num, "%03d", id);
	char mssg[104];
	memset(mssg, NULL, sizeof(mssg));

	char buf[100];
	memset(buf, NULL, sizeof(buf));
	memcpy(buf, &buff[0], len);

	strcat(mssg, "M");
	strcat(mssg, num);
	strcat(mssg, buf);

	// printf("%s, %s\n",buf, mssg);
	int n = sendto(sockfd, mssg, len+4, flags, dest_addr, addrlen);

	if(n<4)
		return -1;

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


int r_recvfrom(int sockfd, char *buff, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
	while(recv_buff_table.front == recv_buff_table.rear) sleep(0.5);

	int front = recv_buff_table.front;
	if(len < strlen(recv_buff_table.mssg[front].buff)){
		pthread_mutex_lock(&recv_buff_lock);

		for(int i=0; i<len; i++)
			buff[i] = recv_buff_table.mssg[front].buff[i];
		*src_addr = recv_buff_table.mssg[front].src_addr;
		*addrlen = sizeof(*src_addr);

		recv_buff_table.front++;

		// for(int i=0; i<size_recv_buf - 1; i++)
		// 	recv_buff_table[i] = recv_buff_table[i+1];

		// size_recv_buf--;
		pthread_mutex_unlock(&recv_buff_lock);

		return (int)len;
	}


	else{
		pthread_mutex_lock(&recv_buff_lock);

		// printf("SIZE: %d, RECV BUFF: %s\n",size_recv_buf, recv_buff_table[0].buff);
		strcpy(buff, recv_buff_table.mssg[front].buff);
		*src_addr = recv_buff_table.mssg[front].src_addr;
		*addrlen = sizeof(*src_addr);

		// for(int i=0; i<size_recv_buf - 1; i++)
		// 	recv_buff_table[i] = recv_buff_table[i+1];

		// size_recv_buf--;
		recv_buff_table.front++;
		pthread_mutex_unlock(&recv_buff_lock);

		return (int)strlen(recv_buff_table.mssg[front].buff);
	}
	return -1;

}


int r_close(int sockfd){
	while(size_unack != 0){
		sleep(0.5);
	}

	free(recv_buff_table.mssg);
	free(unack_mssg_table);
	free(recv_mssg_id_table);

	int n = close(sockfd);
	if(n>0)
		return 0;

	return -1;
}



int dropMessage(float p){
	float n = (float)rand() / RAND_MAX;

	if(n<p)
		return 1;

	return 0;
}
