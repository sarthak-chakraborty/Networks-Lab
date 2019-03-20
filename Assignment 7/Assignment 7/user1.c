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
      
    user1addr.sin_family = AF_INET; 
    user1addr.sin_port = htons(50088); 
    user1addr.sin_addr.s_addr = INADDR_ANY;

    if(r_bind(sockfd, (const struct sockaddr *)&user1addr, sizeof(user1addr)) == -1){
        printf("Binding failed\n");
        exit(0);
    }

    user2addr.sin_family = AF_INET; 
    user2addr.sin_port = htons(50089); 
    user2addr.sin_addr.s_addr = INADDR_ANY;
      
    int n;
    socklen_t len;

    char buff[500];
    printf("Enter a long string(25 to 100 length): ");
    gets(buff);


    printf("User1....\n");

    for(i=0; i<strlen(buff)+1; i++){
        r_sendto(sockfd, (const char *)&(buff[i]), 1, 0, (const struct sockaddr *)&user2addr, sizeof(user2addr));
    }

    printf("\nHello message sent from client\n"); 

           
    int a = r_close(sockfd);

    printf("\nAverage No. of Transmissions: %f\n", (float)a/strlen(buff));
    return 0; 
} 