#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>

int main(int argc, char *argv[]){

	if(argc < 2){
		printf("[Error] Enter domain name in argument.\n");
		exit(0);
	}

	char *domain = argv[1];
	struct hostent *ip;
	struct in_addr **adr;

	ip = gethostbyname(domain);
	if(ip == NULL){
		printf("[Error] Incorrect Domain Name");
		exit(0);
	}

	adr = (struct in_addr **)ip->h_addr_list;
	printf("%s\n",inet_ntoa(*adr[0]));


	int icmp_fd, udp_fd;
	struct sockadd_in snd_addr, rcv_addr;
	int socklen;

	if((icmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
		printf("[Error] Socket creation failed\n");
		exit(0);
	}
	if((udp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0){
		printf("[Error] Socket creation failed\n");
		exit(0);
	}

	snd_addr.sin_family = AF_INET;
	snd_addr.sin_addr.s_addr = INADDR_ANY;
	snd_addr.sin_port = htons(6660);

	rcv_addr.sin_family = AF_INET;
	rcv_addr.sin_addr.s_addr = *adr[0];
	rcv_addr.sin_port = htons(32164);

	if(bind(udp_fd, &snd_addr, sizeof(snd_addr)) < 0){
		printf("[Error] Binding failed\n");
		exit(0);
	}

	

	return 0;
}