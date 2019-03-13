#define SOCK_MRP 1

typedef struct{

}recv_buff;


typedef struct{

}unack_msg_table;


typedef struct{

}recv_msg_id_table;


int r_socket(int domain, int type, int protocol){
	if(type == SOCK_MRP){
		int sockfd = socket(domain, SOCK_DGRAM, protocol);

		pthread_t tid;
		pthread_create(&tid, NULL, runner, (void *)NULL);


	}
	return -1;
}


int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
	int n = bind(sockfd, addr, addrlen);
	if(n>0)
		return 0;

	return -1;
}


int r_sendto(){

}


int r_recvfrom(){

}


int r_close(int sockfd){
	int n = close(sockfd);
	if(n>0)
		return 0;

	return -1;
}


void runner(){

}