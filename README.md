# Networks-Lab

Read the questions(.pdf) files.

### ASSIGNMENT 1
Implementation of a simple UDP socket to transfer words from a file from server to client.


### ASSIGNMENT 2
Implementation of a TCP socket to transfer a file from server to client and count number of bytes and words in the file in client side.


### ASSIGNMENT 3
Implementation of a TCP and UDP socket.  
TCP socket is responsible for transfer of a file word by word from server to client.  
UDP socket is responsible for the response of a DNS client asking for the IP address given a domain name.


### ASSIGNMENT 4
Implementation of active mode FTP, where a server and client creats a TCP server and TCP client each which acts as a control and data socket.


### ASSIGNMENT 5
Implementation of TCP socket where we need to transfer file in a fixed blocks. Use of MSG_WAITALL flag is used in recv() call.


### ASSIGNMENT 6
Implementation of a concurrent server of TCP and UDP like Assignment 3 but in non blocking mode. Both the sockets must be non-blocking. TCP is made unblocking by using the fcnt() function while use of MSG_DONTWAIT is used in the UDP recvfrom() call to make the two sockets non-blocking.


### ASSIGNMENT 7
A reliable connection over an unreliable UDP socket. A socket MRP is designed which will be make a reliable communication using an underlying unreliable UDP socket. In particular, we have to implement a message-oriented, reliable and exactly-once delivery communication layer.
