#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "message_elements.c"

in_addr_t get_query_server();
void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host);

int main( int argc , char *argv[])
{
    unsigned char* hostname = (unsigned char*) "3www6google3com";
    unsigned short qtype = 1;
    extern int errno; 
    
    const int portnum = 53; 
    int socket_file_descriptor;
    struct sockaddr_in server;
    char query[65536];
    char response[65536];

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = inet_addr("8.8.8.8");
    //get_query_server();
    server.sin_port = htons((short) portnum); 

    struct DNS_HEADER *query_header = (struct DNS_HEADER *) &query;
    query_header->id = (unsigned short) htons(getpid());
    printf("id:: %d \n",ntohs(query_header->id));
    query_header->qr = 0; 
    query_header->opcode = 0; 
    query_header->aa = 0; 
    query_header->tc = 0; 
    query_header->rd = 0; 
    query_header->ra = 0; 
    query_header->z = 0;
    query_header->rcode = 0;
    query_header->qd_count = htons(1); 
    query_header->an_count = 0;
    query_header->ns_count = 0;
    query_header->ar_count = 0;

    struct QUESTION *query_question = (struct QUESTION *) &query[sizeof(struct DNS_HEADER)];
    query_question->qname = hostname;
    query_question->qtype = htons(qtype);
    query_question->qclass = htons(1); // Poner una constante para internet
    
    // Creacion del socket - devuelve -1 si da error 
    if((socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("Error al intentar abrir el socket. \n");
        exit(errno); 
    }

    if((sendto(socket_file_descriptor, query, sizeof(struct DNS_HEADER) + sizeof(struct QUESTION), 0, (struct sockaddr*)&server, sizeof(server)))<0)
    {
        perror("Error al intentar enviar un mensaje al servidor. \n");
        exit(errno); 
    }
    printf("Data Sent \n");
    
    int i = sizeof server; 
    struct DNS_HEADER *query_response = (struct DNS_HEADER *) response;
    query_response->id=ntohs(62);
    printf("id:: %d \n",ntohs(query_response->id));

    if(recvfrom(socket_file_descriptor, response, sizeof(response), 0, (struct sockaddr*)&server, (socklen_t*)&i)<0)
    {
        perror("Error al intentar comenzar a atender conexiones. \n");
        exit(errno); 
    }
    printf("[+]Data Received: %li \n", strlen(response));
    printf("[+]Data Received: %i \n", ntohs(response[0]));

    printf("Id:: %d \n",ntohs(query_response->id));
    printf("QR:: %d \n",ntohs(query_response->qr));
    printf("Code:: %d \n",ntohs(query_response->rcode));
    printf("Answer:: %d \n",ntohs(query_response->an_count));
    printf("Qd:: %d \n",ntohs(query_response->qd_count));
    printf("Ns:: %d \n",ntohs(query_response->ns_count));

    return 0;
}

/*
 * Get the query servers from /etc/resolv.conf file on Linux
 * */
in_addr_t get_query_server()
{
    FILE *nameservers_file;
    char readed_line[200], *query_address;

    if((nameservers_file = fopen("/etc/resolv.conf", "r")) == NULL)
    {
        printf("ACA DEBERIAMOS DECIR QUE NO SE PUDO REALIZAR LA CONSULTA PRQUE NO HAY query EN LOS PARAMS NI SABEMOS EL DEFAULT  \n");
    }
    
    while(fgets(readed_line, 200 ,nameservers_file))
    {
        if(readed_line[0] != '#')
        {
            if(strncmp(readed_line, "nameserver", 10) == 0)
            {
                query_address = strtok(readed_line, " ");
                query_address = strtok(NULL, " ");
                return inet_addr(query_address);
            }
        }
    }
    return inet_addr("8.8.8.8"); 
}

/*
 * This will convert www.google.com to 3www6google3com got it :)
 * */
void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host) 
{
    int lock = 0 , i;
    strcat((char*)host,".");
     
    for(i = 0 ; i < strlen((char*)host) ; i++) 
    {
        if(host[i]=='.') 
        {
            *dns++ = i-lock;
            for(;lock<i;lock++) 
            {
                *dns++=host[lock];
            }
            lock++; //or lock=i+1;
        }
    }
    *dns++='\0';
}
