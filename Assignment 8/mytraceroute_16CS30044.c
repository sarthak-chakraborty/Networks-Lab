#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>


#define BUFFER 100




unsigned short ip_checksum(unsigned short *buffer, int size){
    unsigned long cksum=0;
    while(size > 1){
        cksum+=*buffer++;
        size -=sizeof(unsigned short);
    }
    if(size)
        cksum += *(unsigned char *)buffer;

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);
    return (unsigned short)(~cksum);
}



unsigned short udp_checksum(unsigned char *buf, unsigned nbytes){
	unsigned long sum=0;
	uint i;

	/* Checksum all the pairs of bytes first... */
	for (i = 0; i < (nbytes & ~1U); i += 2) {
		sum += (u_int16_t)ntohs(*((u_int16_t *)(buf + i)));
		if (sum > 0xFFFF)
			sum -= 0xFFFF;
	}

	/*
	 * If there's a single byte left over, checksum it, too.
	 * Network byte order is big-endian, so the remaining byte is
	 * the high byte.
	 */
	if (i < nbytes) {
		sum += buf[i] << 8;
		if (sum > 0xFFFF)
			sum -= 0xFFFF;
	}

	return (unsigned short)(sum);
}



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
	// printf("%s\n",inet_ntoa(*adr[0]));


	int icmp_fd, udp_fd, count, ttl = 1;
	struct sockaddr_in udp_addr, icmp_addr, dest_addr;
	int socklen;
	struct iphdr *ip_header;
	struct udphdr *udp_header;
	struct iphdr *recv_ip_header;
	struct icmphdr *icmp_header;
	char buff[BUFFER];
	char mssg[BUFFER];
	fd_set rdfs;
	struct timeval tv;
	time_t t1, t2;


	ip_header = (struct iphdr *)buff;
    udp_header = (struct udphdr *)(buff + sizeof(struct iphdr));
    recv_ip_header = (struct iphdr *)mssg;
    icmp_header = (struct icmphdr *)(mssg + sizeof(struct iphdr));
    memset(buff, 0, BUFFER);


	if((icmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
		printf("[Error] ICMP Socket creation failed\n");
		exit(0);
	}
	if((udp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0){
		printf("[Error] UDP Socket creation failed\n");
		exit(0);
	}


	udp_addr.sin_family = AF_INET;
	udp_addr.sin_addr.s_addr = INADDR_ANY;
	udp_addr.sin_port = htons(6660);

	icmp_addr.sin_family = AF_INET;
	icmp_addr.sin_addr.s_addr = INADDR_ANY;
	icmp_addr.sin_port = htons(6666);

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_ntoa(*adr[0]);
	dest_addr.sin_port = htons(32164);


	if(setsockopt(udp_fd, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0){
		printf("[Error] Setsockopt failed\n");
		exit(0);
	}


	if(bind(udp_fd, &udp_addr, sizeof(udp_addr)) < 0){
		printf("[Error] Binding failed\n");
		exit(0);
	}
	if(bind(icmp_fd, &icmp_addr, sizeof(icmp_addr)) < 0){
		printf("[Error] Binding failed\n");
		exit(0);
	}


	printf("Hop_Count(TTL Value)\tIP Address\tResponse_time\n");
	printf("--------------------\t----------\t-------------\n");

	while(1){
		ip_header->ihl = 5;
	    ip_header->version = 4;
	    ip_header->tos = 0;
	    ip_header->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(buff);
	    ip_header->id = htonl(0);
	    ip_header->frag_off = 0;
	    ip_header->ttl = ttl;
	    ip_header->protocol = 17;    /* TCP. Change to 17 if you want UDP */
	    ip_header->check = 0;
	    ip_header->saddr = udp_addr.sin_addr.s_addr;
	    ip_header->daddr = dest_addr.sin_addr.s_addr;
	    ip_header->check = ip_checksum((unsigned short *)buff, ip_header->tot_len >> 1);

		udp_header->source = udp_addr.sin_port;
		udp_header->dest = dest_addr.sin_port;
		udp_header->len = sizeof(struct udphdr) + sizeof(buff);
		udp_header->check = udp_checksum((unsigned short *)buff, udp_header->len >> 1);;


		sendto(udp_fd, buff, ip_header->tot_len, 0,  (struct sockaddr *)&dest_addr, sizeof(dest_addr));
		t1 = time(NULL);
		

		count = 0;
		while(count < 3){
			FD_ZERO(&rdfs);
	        FD_SET(icmp_fd, &rdfs);
	        tv.tv_sec = 1;
	        tv.tv_usec = 0;
	        int r = select(icmp_fd+1, &rdfs, NULL, NULL, &tv);

	        if(r == -1){
	        	printf("[Error] Select call failed\n");
	        	continue;
	        }
	        if(FD_ISSET(icmp_fd, &rdfs)){
	        	memset(mssg, 0, BUFFER);
				socklen = sizeof(dest_addr);
				int n = recvfrom(icmp_fd, mssg, BUFFER, 0, (struct sockaddr *)&dest_addr, &socklen);
				t2 = time(NULL);
				
				if(icmp_header->type == 11){
					printf("\t%d\t\t%s\t     %d\n",ttl, (char *)inet_ntoa(*(struct in_addr*)&recv_ip_header->saddr), (t2-t1));
					ttl++;
					break;
				}
				else if(icmp_header->type == 3){
					printf("\t%d\t\t%s\t     %d\n",ttl, (char *)inet_ntoa(*(struct in_addr*)&recv_ip_header->saddr), (t2-t1));
					break;
				}
	        }
	        count++;
		}
		if(count == 3){
			printf("\t%d\t\t    *\t\t     *\n",ttl);
			ttl++;
		}
		else if(icmp_header->type == 11) continue;
		else break;
			

	}
	

	return 0;
}