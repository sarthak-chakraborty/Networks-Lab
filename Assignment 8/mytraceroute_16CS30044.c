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


#define BUFFER 52
#define MAX 100
#define MAX_TTL 20


/////////////////////////////////////////////////////////////////////////////
unsigned int checksum(uint16_t *usBuff, int isize)
{
	unsigned int cksum=0;
	for(;isize>1;isize-=2){
	cksum+=*usBuff++;
       }
	if(isize==1){
	 cksum+=*(uint16_t *)usBuff;
        }

	return (cksum);				//calculate the checksum and return 
}

uint16_t check_udp_sum(uint8_t *buffer, int len)			//Standard UDP Checksum Code
{
    unsigned long sum=0;
	struct iphdr *tempI=(struct iphdr *)(buffer);
	struct udphdr *tempH=(struct udphdr *)(buffer+sizeof(struct iphdr));
	tempH->check=0;
	sum=checksum((uint16_t *)&(tempI->saddr) ,8);	//Callinf the checksum function
	sum+=checksum((uint16_t *) tempH,len);

	sum+=ntohs(IPPROTO_UDP+len);

	sum=(sum>>16)+(sum & 0x0000ffff);
	sum=((sum&0)>>16);

	return (uint16_t)(sum);			//Return the checksum
	
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
	printf("\nDestination Domain Requested: ");
	printf("%s (%s), %d hops max\n\n",domain, inet_ntoa(*adr[0]), MAX_TTL);


	int icmp_fd, udp_fd, count, ttl = 1;
	struct sockaddr_in udp_addr, icmp_addr, dest_addr, addr;
	int socklen;
	struct iphdr *ip_header;
	struct udphdr *udp_header;
	struct iphdr *recv_ip_header;
	struct icmphdr *icmp_header;
	char buff[BUFFER + sizeof(struct udphdr) + sizeof(struct iphdr)];
	char mssg[MAX];
	fd_set rdfs;
	struct timeval tv;
	struct timeval t1, t2;


	// Initialize IP and UDP header
	ip_header = (struct iphdr *)buff;
    udp_header = (struct udphdr *)(buff + sizeof(struct iphdr));
    recv_ip_header = (struct iphdr *)mssg;
    icmp_header = (struct icmphdr *)(mssg + sizeof(struct iphdr));
    memset(buff, 0, BUFFER + sizeof(struct udphdr) + sizeof(struct iphdr));


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
	// Bind to the raw sockets 
	if(bind(udp_fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0){
		printf("[Error] Binding failed\n");
		exit(0);
	}
	if(bind(icmp_fd, (struct sockaddr *)&icmp_addr, sizeof(icmp_addr)) < 0){
		printf("[Error] Binding failed\n");
		exit(0);
	}


	// Fill in random bits in payload of UDP
	int i, r;
	for(i=0; i<51; i++){
		r = rand()%26 + 65;
		buff[i + sizeof(struct iphdr) + sizeof(struct udphdr)] = (char)r;
	}
	buff[i + sizeof(struct iphdr) + sizeof(struct udphdr)] = '\0';


	// Set the IP headers
	int id=0;
	ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(sizeof(buff));
    ip_header->id = htonl(id);
    ip_header->frag_off = 0;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->saddr = udp_addr.sin_addr.s_addr;
    ip_header->daddr = inet_addr(inet_ntoa(*adr[0]));

    // Set the UDP Headers
    udp_header->source = udp_addr.sin_port;
    udp_header->dest = htons(32164);
	udp_header->len = htons(sizeof(buff) - sizeof(struct iphdr));
	udp_header->check = check_udp_sum(buff, sizeof(buff) - sizeof(struct iphdr));


	while(ttl<=MAX_TTL){
		ip_header->ttl = ttl;	// Set the ttl field of IP header

		// Send the IP packet to the server
		sendto(udp_fd, buff, ntohs(ip_header->tot_len), 0,  (struct sockaddr *)&dest_addr, sizeof(dest_addr));
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
					struct iphdr *ip_recv_header = (struct iphdr *)(mssg + sizeof(struct iphdr) + sizeof(struct icmphdr));

					if(inet_ntoa(*(struct in_addr*)&ip_recv_header->daddr) == inet_ntoa(*adr[0])){
						printf("Hop_Count(%d)\t%s\t%fms\n",ttl, (char *)inet_ntoa(*(struct in_addr*)&recv_ip_header->saddr), (t2.tv_usec-t1.tv_usec)/1000.0);
						ttl++;
						id++;
						break;
					}
				}
				// If ICMP message is 'DESTINATION UNREACHABLE', break from loop
				else if(icmp_header->type == 3){
					if(inet_ntoa(*(struct in_addr*)&recv_ip_header->saddr) == inet_ntoa(*adr[0])){
						printf("Hop_count(%d)\t%s\t%fms\n",ttl, (char *)inet_ntoa(*(struct in_addr*)&recv_ip_header->saddr), (t2.tv_usec-t1.tv_usec)/1000.0);
						break;
					}	
				}
	        }
	        count++;
		}
		if(count == 3){
			printf("Hop_Count(%d)\t*\t*\n",ttl);
			ttl++;
			id++;
		}
		else if(icmp_header->type == 11) continue;
		else break;		
	}
	close(udp_fd);
	close(icmp_fd);

	return 0;
}