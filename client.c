#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/errno.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dns_response_handler.h"
#include "command_line_manager.h"
#include "query_manager.h"

int main( int argc , char *argv[])
{
    setInputValues(argc,argv);
    
    int socket_file_descriptor;
    struct sockaddr_in server;
    unsigned char *reader;

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = getServer();
    server.sin_port = htons((short) getPort()); 

    // Creacion del socket - devuelve -1 si da error 
    if((socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("Error al intentar abrir el socket. \n");
        exit(errno); 
    }

    unsigned char *qname = sendQuery(server, socket_file_descriptor, getRD(), getHostname(), getQType());

    unsigned char *response = receiveQuery(server, socket_file_descriptor);
    
    struct DNS_HEADER *query_response_header = (struct DNS_HEADER *) response;
    reader = &response[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION_CONSTANT)];
    
    handleResponse(query_response_header, reader, response);

    return 0;
}


