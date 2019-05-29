#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "query_manager.h"
#include "message_elements.h"

unsigned char query[65536], response[65536];

int sendQuery(struct sockaddr_in server, int socket_file_descriptor, char *hostname, unsigned short qtype)
{
    bzero(query, sizeof(query));

    printf("HOSTANAME: %s", hostname);

    struct DNS_HEADER *query_header = (struct DNS_HEADER *) &query;
    query_header->id = (unsigned short) htons(getpid());
    query_header->qr = 0; 
    query_header->opcode = 0; 
    query_header->aa = 0; 
    query_header->tc = 0; 
    query_header->rd = 1;
    query_header->ra = 0; 
    query_header->z = 0;
    query_header->rcode = 0;
    query_header->qd_count = htons(1); 
    query_header->an_count = 0;
    query_header->ns_count = 0;
    query_header->ar_count = 0;

    unsigned char *qname = (unsigned char *) &query[sizeof(struct DNS_HEADER)];
    changeToQNameFormat(qname, hostname);

    struct QUESTION_CONSTANT *q_constant = (struct QUESTION_CONSTANT *) &query[sizeof(struct DNS_HEADER)+ (strlen((const char*)qname) + 1)];
    q_constant->qtype = htons(qtype);
    q_constant->qclass = htons(1);

    if((sendto(socket_file_descriptor, query, sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION_CONSTANT), 0, (struct sockaddr*)&server, sizeof(server)))<0)
    {
        perror("Error al intentar enviar un mensaje al servidor. \n");
        exit(errno); 
    }  

    return strlen(qname) + 1;
}

unsigned char* receiveQuery(struct sockaddr_in server, int socket_file_descriptor)
{
    bzero(response, sizeof(response));

    int i = sizeof server; 
    if(recvfrom(socket_file_descriptor, response, sizeof(response), 0, (struct sockaddr*)&server, (socklen_t*)&i)<0)
    {
        perror("Error al intentar comenzar a atender conexiones. \n");
        exit(errno); 
    }
    return response;
}
