#include <sys/socket.h>

unsigned char* sendQuery(
    struct sockaddr_in server,
    int socket_file_descriptor,
    unsigned char rd,
    char *hostname,
    unsigned short qtype);

unsigned char* receiveQuery(struct sockaddr_in server, int socket_file_descriptor);