# tcp-chat
simple tcp chat written in C using poll for multiplexing
## freatures
- Multiple clients support
- TCP socket communication
- Basic message broadcasting
## build 
gcc server.c -o server
gcc client.c -o client

## usage
Start server
./server <port>

Start client
./client <port> <ip>

Type in one client and see the message appear in the others

## references
- Unix Network Programming – W. Richard Stevens
- Advanced Programming in the UNIX Environment – W. Richard Stevens
