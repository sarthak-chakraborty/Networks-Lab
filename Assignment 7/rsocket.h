#ifndef SOCKET
#define SOCKET
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<pthread.h>
#define T 2
#define PROB 0.05


int r_socket(int, int, int);
int r_bind(int, const struct sockaddr *, socklen_t);
int r_sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
int r_recvfrom(int, void *, size_t, int, struct sockaddr *, soxklen_t *);
int r_close(int);

int dropMessage(float);

#endif