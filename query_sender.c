//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <strings.h>
//#include <netdb.h>
//#include <sys/errno.h>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
//#include <unistd.h>
#include "message_elements.c"

unsigned char query[65536];
extern int errno; 

void sendQuery(
    struct sockaddr_in server,
    int socket_file_descriptor,
    unsigned char rd,
    unsigned char *qname_formated,
    unsigned short qtype)
{
    struct DNS_HEADER *query_header = (struct DNS_HEADER *) &query;
    query_header->id = (unsigned short) htons(getpid());
    query_header->qr = 0; 
    query_header->opcode = 0; 
    query_header->aa = 0; 
    query_header->tc = 0; 
    query_header->rd = rd;
    query_header->ra = 0; 
    query_header->z = 0;
    query_header->rcode = 0;
    query_header->qd_count = htons(1); 
    query_header->an_count = 0;
    query_header->ns_count = 0;
    query_header->ar_count = 0;

    unsigned char *qname = (unsigned char *) &query[sizeof(struct DNS_HEADER)];
    qname = qname_formated;

    struct QUESTION_CONSTANT *q_constant = (struct QUESTION_CONSTANT *) &query[sizeof(struct DNS_HEADER)+ (strlen((const char*)qname) + 1)];
    q_constant->qtype = htons(qtype);
    q_constant->qclass = htons(1); // Poner una constante para internet

    if((sendto(socket_file_descriptor, query, sizeof(struct DNS_HEADER) +  (strlen((const char*)qname)+1) + sizeof(struct QUESTION_CONSTANT), 0, (struct sockaddr*)&server, sizeof(server)))<0)
    {
        perror("Error al intentar enviar un mensaje al servidor. \n");
        exit(errno); 
    }
    printf("Datos enviados.\n");
}
