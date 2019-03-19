#ifndef SOCKET
#define SOCKET
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/select.h>
#include<sys/time.h>
#include<pthread.h>

#define T_SEC 2
#define T_USEC 0
#define PROB 0.3
#define SOCK_MRP 100
#define MAX_SIZE 100


int r_socket(int, int, int);
int r_bind(int, const struct sockaddr *, socklen_t);
int r_sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
int r_recvfrom(int, char *, size_t, int, struct sockaddr *, socklen_t *);
int r_close(int);

int dropMessage(float);


typedef struct{
	char buff[100];
	struct sockaddr src_addr;
}recv_mssg;

typedef struct{
	int front;
	int rear;
	recv_mssg *mssg;
}recv_buff;


typedef struct{
	time_t sent_time;
	struct sockaddr dest_addr;
	int id;
	int flags;
	char mssg[100];
}unack_mssg;


typedef struct{
	int id;
}recv_mssg_id;


#endif