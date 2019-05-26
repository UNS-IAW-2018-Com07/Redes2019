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
#include "command_line_manager.h"

// Tipos de RR 
#define T_A 1 
#define T_NS 2 
#define T_CNAME 5 
#define T_SOA 6 
#define T_PTR 12 
#define T_MX 15 
#define T_LOC 29

in_addr_t getQueryServer();
void readAnswers();
unsigned char* readName();
void printAnswers();

int main( int argc , char *argv[])
{
    setInputValues(argc,argv);

    extern int errno; 

    // Lo que ingresa el usuario
    unsigned char* hostname= getHostname();
    unsigned short qtype = getQType();
    const int portnum = getPort(); 
    
    int socket_file_descriptor;
    struct sockaddr_in aux;
    struct sockaddr_in server;
    unsigned char query[65536], response[65536], *qname, *reader;

    struct RES_RECORD answers[20],auth[20],addit[20];

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = getServer();
    server.sin_port = htons((short) portnum); 

    struct DNS_HEADER *query_header = (struct DNS_HEADER *) &query;
    query_header->id = (unsigned short) htons(getpid());
    query_header->qr = 0; 
    query_header->opcode = 0; 
    query_header->aa = 0; 
    query_header->tc = 0; 
    query_header->rd = getRD();
    query_header->ra = 0; 
    query_header->z = 0;
    query_header->rcode = 0;
    query_header->qd_count = htons(1); 
    query_header->an_count = 0;
    query_header->ns_count = 0;
    query_header->ar_count = 0;

    qname = (unsigned char *) &query[sizeof(struct DNS_HEADER)];
    changeToQNameFormat(qname, hostname);

    struct QUESTION_CONSTANT *q_constant = (struct QUESTION_CONSTANT *) &query[sizeof(struct DNS_HEADER)+ (strlen((const char*)qname) + 1)];
    q_constant->qtype = htons(qtype);
    q_constant->qclass = htons(1); // Poner una constante para internet

    // Creacion del socket - devuelve -1 si da error 
    if((socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("Error al intentar abrir el socket. \n");
        exit(errno); 
    }
    
    /*
    printf("Id:: %d \n",ntohs(query_header->id));
    printf("Puntero Id:: %p \n",&query_header->id);
    
    printf("Rd:: %d \n",query_header->rd);
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
    */

    if((sendto(socket_file_descriptor, query, sizeof(struct DNS_HEADER) +  (strlen((const char*)qname)+1) + sizeof(struct QUESTION_CONSTANT), 0, (struct sockaddr*)&server, sizeof(server)))<0)
    {
        perror("Error al intentar enviar un mensaje al servidor. \n");
        exit(errno); 
    }
    printf("Data Sent \n");
    
    int i = sizeof server; 
    if(recvfrom(socket_file_descriptor, response, sizeof(response), 0, (struct sockaddr*)&server, (socklen_t*)&i)<0)
    {
        perror("Error al intentar comenzar a atender conexiones. \n");
        exit(errno); 
    }

    struct DNS_HEADER *query_response_header = (struct DNS_HEADER *) response;
    reader = &response[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION_CONSTANT)];
    printf("\nThe response contains : ");
    printf("\n %d Questions.",ntohs(query_response_header->qd_count));
    printf("\n %d Answers.",ntohs(query_response_header->an_count));
    printf("\n %d Authoritative Servers.",ntohs(query_response_header->ns_count));
    printf("\n %d Additional records.\n\n",ntohs(query_response_header->ar_count));

/*
    printf("[+]Data Received: %li \n", strlen(response));

    printf("Id:: %d \n",ntohs(query_response->id));

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
    */

    readAnswers(reader, response, query_response_header, answers);

    //print answers
    printf("\nAnswer Records : %d \n" , ntohs(query_response_header->an_count) );
    for(i = 0; i < ntohs(query_response_header->an_count); i++)
    {
        printf("Name : %s ", answers[i].name);
 
        if(ntohs(answers[i].resource_constant->type) == T_A) //IPv4 address
        {
            long *p;
            p = (long*)answers[i].rdata;
            aux.sin_addr.s_addr=(*p); //working without ntohl
            printf("has IPv4 address : %s", inet_ntoa(aux.sin_addr));
        }
         
        if(ntohs(answers[i].resource_constant->type) == T_CNAME) 
        {
            printf("has alias name : %s", answers[i].rdata);
        }
 
        printf("\n");
    }

    return 0;
}

void readAnswers(unsigned char *reader, unsigned char *response, struct DNS_HEADER *query_response_header, struct RES_RECORD *answers)
{ 
    int stop = 0;
 
    for(int i=0;i<ntohs(query_response_header->an_count);i++)
    {
        answers[i].name = readName(reader, response, &stop);
        reader = reader + stop;
 
        answers[i].resource_constant = (struct RES_RECORD_CONSTANT*)reader;
        reader = reader + sizeof(struct RES_RECORD_CONSTANT);

        int answer_type = ntohs(answers[i].resource_constant->type);
        switch(answer_type)
        {
            case 1: // A
            {
                answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource_constant->data_len));
                for(int j = 0; j < ntohs(answers[i].resource_constant->data_len); j++)
                {
                    answers[i].rdata[j]=reader[j];
                }
                answers[i].rdata[ntohs(answers[i].resource_constant->data_len)] = '\0';
                reader = reader + ntohs(answers[i].resource_constant->data_len);
            }; break;
            /*
            case 15: // MX
            {

            }; break;
            case 29: // LOC
            {
                
            }; break;
            */
            default: 
            {
                //No se si sirve para alguno de estos dos 
                answers[i].rdata = readName(reader, response, &stop);
                reader = reader + stop;
            }    
        }
    }
}

unsigned char* readName(unsigned char* reader, unsigned char* buffer, int* count)
{
    unsigned char *name;
    unsigned int p = 0, jumped = 0, offset;
    int i, j;
 
    *count = 1;
    name = (unsigned char*) malloc(256);
 
    // mascara para obtener los dos bits mas significativos
    int bitMask = 49152; // 1100 0000 0000 0000
    name[0]='\0';
 
    //read the names in 3www6google3com format
    while(*reader != 0)
    {
        if(*reader >= 192)
        {
            offset = (*reader)*256 + *(reader+1) - bitMask; 
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++] = *reader;
        }
 
        reader = reader + 1;
 
        if(jumped == 0)
        {
            *count = *count + 1; //if we havent jumped to another location then we can count up
        }
    }
 
    name[p] = '\0'; //caracter terminador
    if(jumped == 1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }
 
    changeFromQNameFormatToNormalFormat(name);
    return name;
}

/*
 * Get the query servers from /etc/resolv.conf file on Linux
 * */
in_addr_t getQueryServer()
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
