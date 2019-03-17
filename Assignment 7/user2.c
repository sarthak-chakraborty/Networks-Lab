#include "rsocket.h"
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if(sockfd < 0){ 
        printf("socket creation failed\n"); 
        exit(1); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(50088); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
      
    int n;
    socklen_t len; 
    char *hello = "CLIENT:HELLO";

    printf("User2....\n");
      
	r_sendto(sockfd, (const char *)hello, strlen(hello), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Hello message sent from client\n"); 

    r_sendto(sockfd, (const char *)hello, strlen(hello), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Hello message sent from client\n"); 
           
    r_close(sockfd); 
    return 0; 
} 