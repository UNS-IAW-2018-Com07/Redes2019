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

void printAnswers(int ans_count, int* preferences, struct RES_RECORD *answers);
void printAuthorities(int ns_count, struct RES_RECORD *auth);
void printAdditional(int ar_count, struct RES_RECORD *addit);

void printResponse(
    struct DNS_HEADER *query_response_header, 
    int* preferences, 
    struct RES_RECORD *answers,
    struct RES_RECORD *auth, 
    struct RES_RECORD *addit)
{
    printf("\n\tLa respuesta contiene: ");
    printf("\n\t %d Consultas.",ntohs(query_response_header->qd_count));
    printf("\n\t %d Respuestas.",ntohs(query_response_header->an_count));
    printf("\n\t %d Servidores autoritativos.",ntohs(query_response_header->ns_count));
    printf("\n\t %d Adicionales.\n",ntohs(query_response_header->ar_count));

    printAnswers(ntohs(query_response_header->an_count), preferences, answers);
    printAuthorities(ntohs(query_response_header->an_count), auth);
    printAdditional(ntohs(query_response_header->an_count), addit);
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

    printf("\n\tCantidad de respuestas: %d \n" , ans_count);
    for(int i = 0; i < ans_count; i++)
    {
        printf("\tNombre de dominio : %s ",answers[i].name);

        switch(ntohs(answers[i].resource_constant->type))
        {
            case T_A: 
            {
                ipv4 = (long*)answers[i].rdata;
                // Casteamos la respuesta a una direccion de internet del campo IN
                aux.sin_addr.s_addr = (*ipv4); 
                // Convertimos el numero correspondiente al campo IN a una respresentacion en ASCII. 
                printf("posee la dirección IPv4: %s\n", inet_ntoa(aux.sin_addr));
            }; break;

            case T_CNAME:
            {
                printf("posee alias: %s\n", answers[i].rdata);
            }; break;

            case T_LOC:
            {
                printf("tiene la siguiente información geográfica: %s\n", answers[i].rdata);
            }; break;

            case T_MX:
            {
                printf("está a cargo del siguiente servidor de correo electrónico: %s", answers[i].rdata);
                printf(" (prioridad = %i)\n", preferences[i]);
            }; break;

            default: 
            {
                printf("ELIMINAR - La consulta es de tipo : %i\n",ntohs(answers[i].resource_constant->type));
            } break;
        }
    }
    printf("\n");
}

void printAuthorities(int ns_count, struct RES_RECORD *auth)
{

}

void printAdditional(int ar_count, struct RES_RECORD *addit)
{

}