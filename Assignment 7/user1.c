#include "rsocket.h"
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
      
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if(sockfd < 0){ 
        printf("socket creation failed\n"); 
        exit(1); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(50088); 
      
    if(r_bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == 0 ){
        printf("bind failed\n"); 
        exit(1); 
    } 
    
    printf("\nUser1....\n");
  
    int n; 
    socklen_t len;
    char buffer[1024]; 
 
    n = r_recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&cliaddr, &len); 
    buffer[n] = '\0'; 
    printf("%s\n", buffer);

    char buf[1024]; 
    n = r_recvfrom(sockfd, buf, 1024, 0, (struct sockaddr *)&cliaddr, &len); 
    buf[n] = '\0'; 
    printf("%s\n", buf);

    r_close(sockfd);
    return 0; 
}