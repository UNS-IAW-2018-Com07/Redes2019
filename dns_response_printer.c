#include <netinet/in.h>
#include <arpa/inet.h>

#include "dns_response_printer.h"

// Tipos de RR 
#define T_A 1 
#define T_NS 2 
#define T_CNAME 5 
#define T_SOA 6 
#define T_PTR 12 
#define T_MX 15 
#define T_LOC 29

void printHeader(struct DNS_HEADER *query_response_header);
void printAnswerCode(int rcode);
void printFlags(struct DNS_HEADER *query_response_header);
void printAnswers(int ans_count, int* preferences, struct RES_RECORD *answers);
void printResourceClass(int class);
void printResourceType(int type);
void printAuthorities(int ns_count, struct RES_RECORD *auth);
void printAdditional(int ar_count, struct RES_RECORD *addit);

void printResponse(
    struct DNS_HEADER *query_response_header, 
    int* preferences, 
    struct RES_RECORD *answers,
    struct RES_RECORD *auth, 
    struct RES_RECORD *addit)
{
    printHeader(query_response_header);

    ntohs(query_response_header->an_count) > 0 ? printAnswers(ntohs(query_response_header->an_count), preferences, answers): NULL;
    ntohs(query_response_header->ns_count) > 0 ? printAuthorities(ntohs(query_response_header->ns_count), auth): NULL;
    ntohs(query_response_header->ar_count) > 0 ? printAdditional(ntohs(query_response_header->ar_count), addit): NULL;
}

void printHeader(struct DNS_HEADER *query_response_header)
{
    printf("\n;; ENCABEZADO\n");
    printf("; Id: %i.",query_response_header->id);
    printAnswerCode(ntohs(query_response_header->rcode));
    printFlags(query_response_header);
    printf("; Consulta/s: %d, ",ntohs(query_response_header->qd_count));
    printf("Respuesta/s: %d, ",ntohs(query_response_header->an_count));
    printf("Servidores autoritativos: %d, ",ntohs(query_response_header->ns_count));
    printf("Adicionales: %d.\n\n",ntohs(query_response_header->ar_count));
}

void printAnswerCode(int rcode)
{
    printf(" Estado: ");
    switch(rcode)
    {
        case 0: printf("OK.\n"); break;
        case 1: printf("Consulta incorrecta.\n"); break;
        case 2: printf("No es posible procesar la consulta.\n"); break;
        case 3: printf("Dominio inexistente.\n"); break;
        case 4: printf("Consulta no soportada.\n"); break;
        case 5: printf("Rehuso por politicas.\n"); break;
        default: printf("RCODE desconocido para el RFC dado por la catedra.\n"); break;
    }
}

void printFlags(struct DNS_HEADER *query_response_header)
{
    query_response_header->rd == 0 ? printf("; Iterativa."): printf("; Recursiva.");
    query_response_header->tc == 0 ? printf(" No truncado."): printf(" Truncado.");
    query_response_header->aa == 0 ? printf(" No autoritativo."): printf(" Autoritativo.");
    query_response_header->ra == 0 ? printf(" Rechaza consultas recursivas.\n"): printf(" Acepta consultas recursivas.\n");
}

/*
 * Imprime en la consola la respuesta de la consulta realizada al DNS.
 * ans_count - Cantidad de repuestas que el DNS proporciono para una determinada 
 *            consulta (en el formato del host).
 * *answers - Puntero al registro que almacena la respuesta del DNS. 
 */

void printAnswers(int ans_count, int* preferences, struct RES_RECORD *answers)
{
    struct sockaddr_in aux;
    long *ipv4;

    printf(";; RESPUESTAS\n");
    for(int i = 0; i < ans_count; i++)
    {
        printf("TTL: %i. ",ntohs(answers[i].resource_constant->ttl));
        printResourceClass(ntohs(answers[i].resource_constant->_class));
        printResourceType(ntohs(answers[i].resource_constant->type));
        printf("%s ",answers[i].name);
        
        switch(ntohs(answers[i].resource_constant->type))
        {
            case T_A: 
            {
                ipv4 = (long*)answers[i].rdata;
                // Casteamos la respuesta a una direccion de internet del campo IN
                aux.sin_addr.s_addr = (*ipv4); 
                // Convertimos el numero correspondiente al campo IN a una respresentacion en ASCII. 
                printf("posee la dirección IPv4: %s\n\n", inet_ntoa(aux.sin_addr));
            }; break;

            case T_CNAME:
            {
                printf("posee alias: %s\n\n", answers[i].rdata);
            }; break;

            case T_LOC:
            {
                printf("tiene la siguiente información geográfica: %s\n\n", answers[i].rdata);
            }; break;

            case T_MX:
            {
                printf("está a cargo del siguiente servidor de correo electrónico: %s", answers[i].rdata);
                printf(" (prioridad = %i)\n\n", preferences[i]);
            }; break;
        }
    }
}

void printResourceClass(int class)
{
    class == 1 ? printf("Clase: IN. "): printf("Clase: ?. ");
}

void printResourceType(int type)
{
    switch(type)
    {
        case T_A: printf("Tipo: A.\n"); break;
        case T_NS: printf("Tipo: NS\n"); break;
        case T_CNAME: printf("Tipo: CNAME\n"); break;
        case T_SOA: printf("Tipo: SOA\n"); break;
        case T_PTR: printf("Tipo: PTR\n"); break;
        case T_MX: printf("Tipo: MX\n"); break;
        case T_LOC: printf("Tipo: LOC\n"); break;
        default: printf("Tipo: ?\n"); break;
    } 
}

void printAuthorities(int ns_count, struct RES_RECORD *auth)
{
    printf(";; AUTORITATIVOS\n");
    for(int i = 0; i < ns_count; i++)
    {
        printf("TTL: %i. ",ntohs(auth[i].resource_constant->ttl));
        printResourceClass(ntohs(auth[i].resource_constant->_class));
        printResourceType(ntohs(auth[i].resource_constant->type)); 
        printf("%s ",auth[i].name);

        if( ntohs(auth[i].resource_constant->type) == T_NS ||
            ntohs(auth[i].resource_constant->type) == T_SOA )
        {
            printf("posee nameserver: %s\n", auth[i].rdata);
        }
    }
}

void printAdditional(int ar_count, struct RES_RECORD *addit)
{
    struct sockaddr_in aux;
    long *ipv4;

    printf(";; ADICIONALES\n");
    for(int i = 0; i < ar_count; i++)
    {
        printf("TTL: %i. ",ntohs(addit[i].resource_constant->ttl));
        printResourceClass(ntohs(addit[i].resource_constant->_class));
        printResourceType(ntohs(addit[i].resource_constant->type)); 
        printf("%s ",addit[i].name);

        if(ntohs(addit[i].resource_constant->type) == T_A)
        {
            ipv4 = (long*) addit[i].rdata;
            aux.sin_addr.s_addr = (*ipv4);
            printf("posee la dirección IPv4: %s\n",inet_ntoa(aux.sin_addr));
        }
    }
}