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
    unsigned char* hostname= "www.google.com";
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

    unsigned char *qname = (unsigned char *) &query[sizeof(struct DNS_HEADER)];
    ChangetoDnsNameFormat(qname, hostname);

    struct QUESTION_CONSTANT *q_constant = (struct QUESTION_CONSTANT *) &query[sizeof(struct DNS_HEADER)+ (strlen((const char*)qname) + 1)];
    q_constant->qtype = htons(qtype);
    q_constant->qclass = htons(1); // Poner una constante para internet
    //struct QUESTION *query_question = (struct QUESTION *) &query[sizeof(struct DNS_HEADER)];
    //unsigned char *qname = malloc(sizeof(hostname)+2);
    //qname = query_question->qname;
    //ChangetoDnsNameFormat(qname, hostname);
    //query_question->qname = hostname;
    //&query_question->qtype = 
    //query_question->qtype = htons(qtype);
    //query_question->qclass = htons(1); // Poner una constante para internet

    // Creacion del socket - devuelve -1 si da error 
    if((socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("Error al intentar abrir el socket. \n");
        exit(errno); 
    }

    printf("Id:: %d \n",ntohs(query_header->id));
    printf("Puntero Id:: %p \n",&query_header->id);
    
    printf("Rd:: %d \n",ntohs(query_header->rd));
    printf("TC:: %d \n",ntohs(query_header->tc));
    printf("AA:: %d \n",ntohs(query_header->aa));
    printf("opcode:: %d \n",ntohs(query_header->opcode));
    printf("QR:: %d \n",ntohs(query_header->qr));

    printf("Rcode:: %d \n",ntohs(query_header->rcode));
    printf("Z:: %d \n",ntohs(query_header->z));
    printf("RA:: %d \n",ntohs(query_header->ra));

    printf("QdCount:: %d \n",ntohs(query_header->qd_count));
    printf("AnswerCount:: %d \n",ntohs(query_header->an_count));
    printf("NsCount:: %d \n",ntohs(query_header->ns_count));
    printf("ArCount:: %d \n",ntohs(query_header->ar_count));

    if((sendto(socket_file_descriptor, query, sizeof(struct DNS_HEADER) +  (strlen((const char*)qname)+1) + sizeof(struct QUESTION_CONSTANT), 0, (struct sockaddr*)&server, sizeof(server)))<0)
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

    printf("Id:: %d \n",ntohs(query_response->id));
    printf("Puntero Id:: %p \n",&query_response->id);

    printf("Rd:: %d \n",ntohs(query_response->rd));
    printf("TC:: %d \n",ntohs(query_response->tc));
    printf("AA:: %d \n",ntohs(query_response->aa));
    printf("opcode:: %d \n",ntohs(query_response->opcode));
    printf("QR - Ntohs:: %d \n",ntohs(query_response->qr));
    printf("QR - Htons:: %d \n",htons(query_response->qr));
    printf("QR - Sin Nada:: %d \n",query_response->qr);

    printf("Rcode - Htons:: %d \n",ntohs(query_response->rcode));
    printf("Rcode - Htons:: %d \n",htons(query_response->rcode));
    printf("Rcode - Sin nada:: %d \n",query_response->rcode);
    printf("Z:: %d \n",ntohs(query_response->z));
    printf("RA:: %d \n",ntohs(query_response->ra));

    printf("QdCount:: %d \n",ntohs(query_response->qd_count));
    printf("AnswerCount:: %d \n",ntohs(query_response->an_count));
    printf("NsCount:: %d \n",ntohs(query_response->ns_count));
    printf("ArCount:: %d \n",ntohs(query_response->ar_count));
    
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
    unsigned char *host_copy = malloc(sizeof(strlen(host) + 1));
    strcpy(host_copy,host);

    int lock = 0 , i;
    strcat((char *) host_copy,".");
     
    for(i = 0 ; i < strlen((char*)host_copy) ; i++) 
    {
        if(host_copy[i]=='.') 
        {
            *dns++ = i-lock;
            for(;lock<i;lock++) 
            {
                *dns++=host_copy[lock];
            }
            lock++; //or lock=i+1;
        }
    }
    *dns++='\0';
    free(host_copy);
}
