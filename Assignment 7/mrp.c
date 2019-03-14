#include "rsocket.h"

recv_buff *recv_buff_table;
unack_mssg *unack_mssg_table;
recv_mssg_id *recv_mssg_id_table;

static int id = 0;
int index = 0;


int r_socket(int domain, int type, int protocol){
	if(type == SOCK_MRP){
		int sockfd = socket(domain, SOCK_DGRAM, protocol);

		pthread_t tid;
		pthread_create(&tid, NULL, runner, (void *)&sockfd);

		recv_buff_table = (recv_buff *)malloc(100*sizeof(recv_buff));
		unack_mssg_table = (unack_mssg *)malloc(100*sizeof(unack_mssg));
		recv_mssg_id_table = (recv_mssg_id *)malloc(100*sizeof(recv_mssg_id));

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
	char num[3];
	sprintf(num, "%d", id);
	char *mssg;
	strcat(mssg, 'M');
	strcat(mssg, num);
	strcat(mssg, buff);
	int n = send(sockfd, mssg, len+4, flags, dest_addr, addrlen);

	if(n<4)
		return -1;

	time(&(unack_mssg_table[index].sent_time));
	unack_mssg_table[index].dest_addr = *dest_addr;
	unack_mssg_table[index].id = id;
	unack_mssg_table[index].mssg = buff;

	id++;
	index++;

	return 0;
}


int r_recvfrom(int sockfd, void *buff, size_t len, int flags, const struct sockaddr *src_addr, socklen_t *addrlen){
	while(recv_buff_table[0] == NULL) sleep(2);

	if(len < strlen(recv_buff_table[0])){
		for(int i=0; i<len; i++){
			buff[i] = recv_buff_table[0].buff[i];
		}
		*src_addr = recv_buff_table[0].src_addr;
		return len;
	}
	else{
		strcpy(buff, recv_buff_table[0].buff);
		*src_addr = recv_buff_table[0].src_addr;
		return strlen(recv_buff_table[0].buff);
	}

}


int r_close(int sockfd){
	int n = close(sockfd);

	while(index == 0){
		sleep(2);
		continue;
	}

	free(recv_buff_table);
	free(unack_mssg_table);
	free(recv_mssg_id_table);

	if(n>0)
		return 0;

	return -1;
}


void *runner(void *parameters){
	int sockfd = (int)*parameters;
	fd_set readfs;
	struct timeval t;
	char buf[104];
	struct sockaddr_in cliaddr;
	socklen_t clilen;

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
		if(FD_ISSET(sockfd, &readfs)){
			recv(sokcfd, buf, 104, 0, (struct sockaddr *)&cliaddr, &clilen);
			if(buf[0] == 'M'){

			}
			else if(buf[0] == 'A'){
				
			}
		}
		else{
			time_t t;
			time(&t);
			int i;
			for(i=0; i<index; i++){
				if(t - unack_mssg_table[i].sent_time >= T_SEC){
					//send the essage again
				}
			}
		}
	}
}