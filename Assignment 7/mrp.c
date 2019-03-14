#define SOCK_MRP 1

typedef struct{
	char buff[100];
}recv_buff;
recv_buff *recv_buff_table;


typedef struct{
	time_t sent_time;
	struct sockaddr *dest_addr;
	int id;
	char mssg[100];
}unack_mssg;
unack_mssg *unack_mssg_table;


typedef struct{
	int id;
}recv_mssg_id;
recv_mssg_id *recv_mssg_id_table;


static int id = 0;
int index = 0;


int r_socket(int domain, int type, int protocol){
	if(type == SOCK_MRP){
		int sockfd = socket(domain, SOCK_DGRAM, protocol);

		pthread_t tid;
		pthread_create(&tid, NULL, runner, (void *)NULL);

		recv_buff_table = (recv_buff *)malloc(100*sizeof(recv_buff));
		unack_mssg_table = (unack_mssg *)malloc(100*sizeof(unack_mssg));
		recv_mssg_id_table = (recv_mssg_id *)malloc(100*sizeof(recv_mssg_id));
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
	char num[3];
	sprintf(num, "%d", id);
	char *mssg;
	strcat(mssg, num);
	strcat(mssg, buff);
	int n = send(sockfd, mssg, len+3, flags, dest_addr, addrlen);

	if(n<0)
		return -1;

	time(&(unack_mssg_table[index].sent_time));
	unack_mssg_table[index].dest_addr = dest_addr;
	unack_mssg_table[index].id = id;
	unack_mssg_table[index].mssg = buff;

	id++;
	index++;

	return 0;
}


int r_recvfrom(int sockfd, void *buff, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t *addrlen){
	while(recv_buff_table[0] == NULL) sleep(2);

	if(len < strlen(recv_buff_table[0])){
		for(int i=0; i<len; i++){
			buff[i] = recv_buff_table[0][i];
		}
		return len;
	}
	else{
		strcpy(buff, recv_buff_table[0]);
		return strlen(recv_buff_table[0]);
	}

}


int r_close(int sockfd){
	int n = close(sockfd);

	free(recv_buff_table);
	free(unack_mssg_table);
	free(recv_mssg_id_table);

	if(n>0)
		return 0;

	return -1;
}


void runner(){

}