#include <sys/socket.h>

int sendQuery(struct sockaddr_in server, int socket_file_descriptor, char *hostname, unsigned short qtype);

unsigned char* receiveQuery(struct sockaddr_in server, int socket_file_descriptor);