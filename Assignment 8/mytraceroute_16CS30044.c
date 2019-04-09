#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
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
#define MAX 100


/////////////////////////////////////////////////////////////////////////////
// Function for checksum of UDP
unsigned short udp_checksum(unsigned char *buf, unsigned nbytes){
	unsigned long sum=0;
	uint i;

	for(i=0; i<(nbytes & ~1U); i+=2) {
		sum += (u_int16_t)ntohs(*((u_int16_t *)(buf + i)));
		if(sum > 0xFFFF)
			sum -= 0xFFFF;
	}
	if(i < nbytes){
		sum += buf[i] << 8;
		if(sum > 0xFFFF)
			sum -= 0xFFFF;
	}
	return (unsigned short)(sum);
}



int main(int argc, char *argv[]){
	// Check if domain name is provided
	if(argc < 2){
		printf("[Error] Enter domain name in argument.\n");
		exit(0);
	}

	char *domain = argv[1];
	struct hostent *ip;
	struct in_addr **adr;

	// DNS query for IP address of the domain
	ip = gethostbyname(domain);
	if(ip == NULL){
		printf("[Error] Incorrect Domain Name");
		exit(0);
	}

	adr = (struct in_addr **)ip->h_addr_list;


	int icmp_fd, udp_fd, count, ttl = 1;
	struct sockaddr_in udp_addr, icmp_addr, dest_addr, addr;
	int socklen;
	struct iphdr *ip_header;
	struct udphdr *udp_header;
	struct iphdr *recv_ip_header;
	struct icmphdr *icmp_header;
	char buff[52 + sizeof(struct udphdr) + sizeof(struct iphdr)];
	char mssg[MAX];
	fd_set rdfs;
	struct timeval tv;
	struct timeval t1, t2;


	// Initialize IP and UDP header
	ip_header = (struct iphdr *)buff;
    udp_header = (struct udphdr *)(buff + sizeof(struct iphdr));
    recv_ip_header = (struct iphdr *)mssg;
    icmp_header = (struct icmphdr *)(mssg + sizeof(struct iphdr));
    memset(buff, 0, BUFFER);


    // Create raw sockets
	if((icmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
		printf("[Error] ICMP Socket creation failed\n");
		exit(0);
	}
	if((udp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0){
		printf("[Error] UDP Socket creation failed\n");
		exit(0);
	}


	// Set the addresses of the sockets
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_addr.s_addr = INADDR_ANY;
	udp_addr.sin_port = htons(6660);

	icmp_addr.sin_family = AF_INET;
	icmp_addr.sin_addr.s_addr = INADDR_ANY;
	icmp_addr.sin_port = htons(6666);

	// Server address where the query is to be made
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*adr[0]));
	dest_addr.sin_port = htons(32164);


	// Set socket option for including headers
	if(setsockopt(udp_fd, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0){
		printf("[Error] Setsockopt failed\n");
		exit(0);
	}
	if(setsockopt(icmp_fd, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0){
		printf("[Error] Setsockopt failed\n");
		exit(0);
	}
	// Bind to the raw sockets 
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


	// Set the IP headers
	int id=0;
	ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = sizeof(buff);
    ip_header->id = htonl(id);
    ip_header->frag_off = 0;
    ip_header->protocol = IPPROTO_IP;
    ip_header->saddr = udp_addr.sin_addr.s_addr;
    ip_header->daddr = inet_addr(inet_ntoa(*adr[0]));

    // Set the UDP Headers
    udp_header->source = udp_addr.sin_port;
    udp_header->dest = htons(32164);
	udp_header->len = sizeof(buff) - sizeof(struct iphdr);
	udp_header->check = udp_checksum((unsigned short *)buff, udp_header->len >> 1);


	while(1){
		ip_header->ttl = ttl;	// Set the ttl field of IP header

		// Fill in random bits in payload of UDP
		int i, r;
		for(i=0; i<51; i++){
			r = rand()%26 + 65;
			buff[i + sizeof(struct iphdr) + sizeof(struct udphdr)] = (char)r;
		}
		buff[i + sizeof(struct iphdr) + sizeof(struct udphdr)] = '\0';

		// Send the IP packet to the server
		sendto(udp_fd, buff, ip_header->tot_len, 0,  (struct sockaddr *)&dest_addr, sizeof(dest_addr));
		gettimeofday(&t1, NULL);
		

		// Wait on a select call for timeout or a reply
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
	        // If relpy from the socket
	        if(FD_ISSET(icmp_fd, &rdfs)){
	        	memset(mssg, 0, MAX);
				socklen = sizeof(addr);
				int n = recvfrom(icmp_fd, mssg, MAX, 0, (struct sockaddr *)&addr, &socklen);
				gettimeofday(&t2, NULL);
				
				// If ICMP message is of 'TIME LIMIT EXCEEDED', increase ttl and loop again
				if(icmp_header->type == 11){
					printf("\t%d\t\t%s\t     %dms\n",ttl, (char *)inet_ntoa(*(struct in_addr*)&recv_ip_header->saddr), (t2.tv_usec-t1.tv_usec)/1000);
					ttl++;
					id++;
					break;
				}
				// If ICMP message is 'DESTINATION UNREACHABLE', break from loop
				else if(icmp_header->type == 3){
					printf("\t%d\t\t%s\t     %dms\n",ttl, (char *)inet_ntoa(*(struct in_addr*)&recv_ip_header->saddr), (t2.tv_usec-t1.tv_usec)/1000);
					break;
				}
	        }
	        count++;
		}
		if(count == 3){
			printf("\t%d\t\t    *\t\t     *\n",ttl);
			ttl++;
			id++;
		}
		else if(icmp_header->type == 11) continue;
		else break;		
	}
	return 0;
}