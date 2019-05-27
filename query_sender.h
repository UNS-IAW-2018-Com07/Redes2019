unsigned char* sendQuery(
    struct sockaddr_in server,
    int socket_file_descriptor,
    unsigned char rd,
    unsigned char *hostname,
    unsigned short qtype);