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
void printLine(struct RES_RECORD rrecord, int *unaccepted_responses, int preferences);
int isTypeAccepted(int type); 

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

/* 
 * Imprime por consola el encabezado de la consulta realizada al DNS.
 * *query_response_header - Puntero al inicio del encabezado de la respuesta.
 */
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

/* 
 * Imprime por consola el estado de la consulta realizada al DNS segun el RFC 1035 (Acepta del 0-5)
 * rcode - Codigo de respuesta de la consulta realizada. 
 */
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

/* 
 * Imprime por consola las flags que se encuentran en el encabezado: Recursion Desired (rd), TrunCation (tc), Recursion Available (ra) y Authoritative Answer (aa). 
 * *query_response_header - Puntero al inicio del encabezado de la respuesta.
 */
void printFlags(struct DNS_HEADER *query_response_header)
{
    query_response_header->rd == 0 ? printf("; Iterativa."): printf("; Recursiva.");
    query_response_header->tc == 0 ? printf(" No truncado."): printf(" Truncado.");
    query_response_header->aa == 0 ? printf(" No autoritativo."): printf(" Autoritativo.");
    query_response_header->ra == 0 ? printf(" Rechaza consultas recursivas.\n"): printf(" Acepta consultas recursivas.\n");
}

/*
 * Imprime en la consola la respuesta almacenada en la seccion ASNWER de la consulta realizada al DNS.
 * ans_count - Cantidad de repuestas (seccion ANSWER) que el DNS proporciono para una determinada consulta.
 * *answers - Puntero a los registros que almacenan las respuestas de la seccion ANSWER.
 * *preferences - Puntero a ans_count enteros que almacenan el valor correspondiente al campo preference de una respuesta MX.
 */
void printAnswers(int ans_count, int* preferences, struct RES_RECORD *answers)
{
    int unaccepted_responses = 0;
    printf(";; RESPUESTAS\n");
    for(int i = 0; i < ans_count; i++)
    {
        printLine(answers[i], &unaccepted_responses, preferences[i]);
    }
    (unaccepted_responses > 0) ? printf("; Respuestas sin visualizar: %i\n\n",unaccepted_responses): printf("\n");
}

/*
 * Imprime en la consola la respuesta almacenada en la seccion AUTHORITY de la consulta realizada al DNS.
 * ns_count - Cantidad de repuestas de RR autoritativos que el DNS proporciono para una determinada consulta.
 * *auth - Puntero a los registros que almacenan las respuestas de la seccion AUTHORITY.
 */
void printAuthorities(int ns_count, struct RES_RECORD *auth)
{
    int unaccepted_responses = 0;
    printf(";; AUTORITATIVOS\n");
    for(int i = 0; i < ns_count; i++)
    {
        printLine(auth[i], &unaccepted_responses, 0);
    }
    (unaccepted_responses > 0) ? printf("; Autoritativos sin visualizar: %i\n\n",unaccepted_responses): printf("\n");
}

/*
 * Imprime en la consola la respuesta almacenada en la seccion ADDITIONAL de la consulta realizada al DNS.
 * ar_count - Cantidad de repuestas de RR adicionales que el DNS proporciono para una determinada consulta.
 * *addit - Puntero a los registros que almacenan las respuestas de la seccion ADDITIONAL.
 */
void printAdditional(int ar_count, struct RES_RECORD *addit)
{
    int unaccepted_responses = 0;
    printf(";; ADICIONALES\n");
    for(int i = 0; i < ar_count; i++)
    {
       printLine(addit[i], &unaccepted_responses, 0);
    }
    (unaccepted_responses > 0) ? printf("; Adicionales sin visualizar: %i\n\n",unaccepted_responses): printf("\n");
}

/*
 * Imprime en la consola la respuesta almacenada en un RR.
 * rrecord - RR a imprimir por pantalla.
 * preferences - Si el RR es de tipo MX entonces se imprimira el valor almacenado en esta variable. 
 * *unaccepted_responses - Puntero a la cantidad de registros que poseen tipo de respuesta incompatible con el programa. Si el RR actual no es compatible entonces se incrementa en uno lo apuntado por esta variable.
 */
void printLine(struct RES_RECORD rrecord, int *unaccepted_responses, int preferences)
{
    if(isTypeAccepted(ntohs(rrecord.resource_constant->type)) == 1)
    {
        printf("%s.\t\t",rrecord.name);
        printf("%i\t",ntohs(rrecord.resource_constant->ttl));
        (ntohs(rrecord.resource_constant->_class) == 1) ? printf("IN\t"): printf("?\t");
        printResourceType(ntohs(rrecord.resource_constant->type));

        switch(ntohs(rrecord.resource_constant->type))
        {
            case T_A: 
            {
                long *ipv4 = (long*)rrecord.rdata;
                // Casteamos la respuesta a una direccion de internet del campo IN
                struct sockaddr_in aux;
                aux.sin_addr.s_addr = (*ipv4); 
                // Convertimos el numero correspondiente al campo IN a una respresentacion en ASCII. 
                printf("%s\n", inet_ntoa(aux.sin_addr));
            }; break;
            case T_MX:
            {
                printf("%s ", rrecord.rdata);
                printf("(prioridad = %i)\n", preferences);
            }; break;
            default: 
            {
                printf("%s\n", rrecord.rdata);
            }
        }
    }
    else
    {
        (*unaccepted_responses)++;
    }
}

/*
 * Determina si el tipo es compatible con el programa. Retorna 1 cuando es compatible y 0 cuando no lo es. 
 * type - Entero que representa el tipo del RR. 
 */
int isTypeAccepted(int type)
{
    int result = 0;
    if(type == T_A || type == T_NS ||  type == T_SOA || type == T_PTR ||
       type == T_CNAME || type == T_MX || type == T_LOC)
    {
        result = 1;
    }
    return result; 
}

/*
 * Imprime por consola el tipo del RR en formato legible para el usuario. 
 * type - Entero que representa el tipo del RR. 
 */
void printResourceType(int type)
{
    switch(type)
    {
        case T_A: printf("A\t"); break;
        case T_NS: printf("NS\t"); break;
        case T_CNAME: printf("CNAME\t"); break;
        case T_PTR: printf("PTR\t"); break;
        case T_SOA: printf("SOA\t"); break;
        case T_MX: printf("MX\t"); break;
        case T_LOC: printf("LOC\t"); break;
    } 
}