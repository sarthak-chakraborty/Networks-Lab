#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/ip.h> /* for ipv4 header */
#include <linux/udp.h> /* for upd header */
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#define MSG_SIZE 2048
#define LISTEN_PORT 8080
// #define LISTEN_IP "127.0.0.1""
#define LISTEN_IP "0.0.0.0"

int main(){
	int rawfd, udpfd;
	struct sockaddr_in saddr_raw, saddr_udp;
	struct sockaddr_in raddr;
	int saddr_raw_len, saddr_udp_len;
	int raddr_len;
	char msg[MSG_SIZE];
	int msglen;
	pid_t pid = fork();
	if(pid == 0){
		struct iphdr hdrip;
		struct udphdr hdrudp;
		int iphdrlen = sizeof(hdrip);
		int udphdrlen = sizeof(hdrudp);
		rawfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
		if(rawfd < 0){
			perror("raw socket");
			exit(__LINE__);
		}
		saddr_raw.sin_family = AF_INET;
		saddr_raw.sin_port = htons(LISTEN_PORT);
		saddr_raw.sin_addr.s_addr = INADDR_ANY; //inet_addr(LISTEN_IP);
		saddr_raw_len = sizeof(saddr_raw);
		if(bind(rawfd, (struct sockaddr*) &saddr_raw, saddr_raw_len) < 0){
			perror("raw bind");
			exit(__LINE__);
		}
		while(1){
			raddr_len = sizeof(raddr);
			msglen = recvfrom(rawfd, msg, MSG_SIZE, 0, (struct sockaddr *)&raddr, &raddr_len);
			if (msglen <= 0) //ignoring all the errors is not a good idea.
				continue;
			hdrip = *((struct iphdr *) msg);
			hdrudp = *((struct udphdr *) (msg + iphdrlen));
			if(hdrudp.dest != saddr_raw.sin_port)
				continue;
			msg[msglen] = 0;
			printf("RAW socket: ");
			printf("hl: %d, version: %d, ttl: %d, protocol: %d",hdrip.ihl, hdrip.version, hdrip.ttl, hdrip.protocol);
			printf(", src: %s", inet_ntoa(*((struct in_addr *)&hdrip.saddr)));
			printf(", dst: %s", inet_ntoa(*((struct in_addr *)&hdrip.daddr)));
			printf("\nRAW socket: \tUdp sport: %d, dport: %d, len:%d",ntohs(hdrudp.source), ntohs(hdrudp.dest),ntohs(hdrudp.len));
			printf("\n%d, %d, %d, %d\n",sizeof(hdrudp.source),sizeof(hdrudp.dest),sizeof(hdrudp.len),sizeof(hdrudp.check));
			printf("\nRAW socket: \tfrom: %s:%d",inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
			printf("\nRaw Socket: \tUDP payload: %s",msg+iphdrlen+udphdrlen);
			printf("\n");
		}
		close(rawfd);
	}
	else{
		udpfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(udpfd < 0){
			perror("udp socket");
			exit(__LINE__);
		}
		saddr_udp.sin_family = AF_INET;
		saddr_udp.sin_port = htons(LISTEN_PORT);
		saddr_udp.sin_addr.s_addr = INADDR_ANY; //inet_addr(LISTEN_IP);
		saddr_udp_len = sizeof(saddr_udp);
		if(bind(udpfd, (struct sockaddr*) &saddr_udp, saddr_udp_len) < 0){
			perror("raw bind");
			exit(__LINE__);
		}
		while(1){
			raddr_len = sizeof(raddr);
			msglen = recvfrom(udpfd, msg, MSG_SIZE, 0, (struct sockaddr *)&raddr, &raddr_len);
			msg[msglen] = 0;
			printf("UDP: recv len: %d, recvfrom: %s:%d\n", msglen,inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
			printf("UDP: payload: %s\n", msg);
		}
		close(udpfd);
	}
	return 0;
}