#include "rsocket.h"
  
int main() { 
    int sockfd, i; 
    struct sockaddr_in user1addr, user2addr; 
      
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if(sockfd < 0){ 
        printf("socket creation failed\n"); 
        exit(1); 
    } 
      
    memset(&user1addr, 0, sizeof(user1addr)); 
    memset(&user2addr, 0, sizeof(user2addr)); 
      
    user1addr.sin_family    = AF_INET; 
    user1addr.sin_addr.s_addr = INADDR_ANY; 
    user1addr.sin_port = htons(50088);

    user2addr.sin_family    = AF_INET; 
    user2addr.sin_addr.s_addr = INADDR_ANY; 
    user2addr.sin_port = htons(50089); 
      
    if(r_bind(sockfd, (const struct sockaddr *)&user2addr, sizeof(user2addr)) == -1 ){
        printf("bind failed\n"); 
        exit(1); 
    } 
    
    printf("\nUser2....\n");
  
    int n; 
    socklen_t len;
    char c;

    while(1){
        fflush(stdout);
        len = sizeof(user1addr);
        r_recvfrom(sockfd, &c, 1, 0, (struct sockaddr *)&user1addr, &len);
        printf("%c", c);
    }

    printf("Over\n");
    r_close(sockfd);
    return 0; 
}