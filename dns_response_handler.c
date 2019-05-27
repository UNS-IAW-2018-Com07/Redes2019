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
#include "location_reader.h"

void readAnswers(int ans_count, unsigned char *reader, unsigned char *response, struct RES_RECORD *answers);
unsigned char* readName(unsigned char *reader, unsigned char *response, int *count); 
void printAnswers(int ans_count, struct RES_RECORD *answers);
void printAuthority(int ns_count, struct RES_RECORD *auth);
void printAdditional(int ar_count, struct RES_RECORD *addit);

int preferences[20];

void handleResponse(struct DNS_HEADER *query_response_header, unsigned char *reader, unsigned char *response)
{
    struct RES_RECORD answers[20];

    printf("\n\tLa respuesta contiene: ");
    printf("\n\t %d Consultas.",ntohs(query_response_header->qd_count));
    printf("\n\t %d Respuestas.",ntohs(query_response_header->an_count));
    printf("\n\t %d Servidores autoritativos.",ntohs(query_response_header->ns_count));
    printf("\n\t %d Adicionales.\n",ntohs(query_response_header->ar_count));
   
    readAnswers(ntohs(query_response_header->an_count), reader, response, answers);
    printAnswers(ntohs(query_response_header->an_count), answers);
}

/*
 * Lee las respuestas proporcionadas por el DNS de acuerdo a su tipo: A, MX, LOC, CNAME, entre otros. 
 * ans_count - Cantidad de repuestas que el DNS proporciono para una determinada 
 *             consulta (en el formato del host).
 * *reader   - Puntero a la seccion ANSWER de la consulta realizada al DNS. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *answers  - Puntero al registro que almacenara las respuestas del DNS.
 */

void readAnswers(int ans_count, unsigned char *reader, unsigned char *response, struct RES_RECORD *answers)
{ 
    int stop = 0;
 
    for(int i = 0; i < ans_count; i++)
    {
        answers[i].name = readName(reader, response, &stop);
        reader = reader + stop;
 
        answers[i].resource_constant = (struct RES_RECORD_CONSTANT*)reader;
        reader = reader + sizeof(struct RES_RECORD_CONSTANT);

        int answer_type = ntohs(answers[i].resource_constant->type);
        switch(answer_type)
        {
            case T_A:
            {
                answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource_constant->data_len));
                for(int j = 0; j < ntohs(answers[i].resource_constant->data_len); j++)
                {
                    answers[i].rdata[j]=reader[j];
                }
                answers[i].rdata[ntohs(answers[i].resource_constant->data_len)] = '\0';
                reader = reader + ntohs(answers[i].resource_constant->data_len);
            }; break;
            case T_MX:
            {
                preferences[i] = (int)reader[1];
                reader = reader + 2;
                answers[i].rdata = readName(reader, response, &stop);
                reader = reader + stop;
            }; break;
            case T_LOC:
            {
                answers[i].rdata = (unsigned char*)loc_ntoa(reader, NULL);
                reader = reader + ntohs(answers[i].resource_constant->data_len);
            }; break;
            default: 
            {
                //No se si sirve para alguno de estos dos 
                answers[i].rdata = readName(reader, response, &stop);
                reader = reader + stop;
            }    
        }
    }
}

/*
* El DNS utiliza un esquema de compresion en donde elimina la repeticion de nombre de domino en los campos 
* NAME, QNAME y RDATA. El nombre de dominio es reemplazado por un puntero a una ocurrencia anterior de dicho 
* nombre. 
* El puntero (2 bytes) posee los dos primeros bits encendidos (por lo que tiene que el primero octeto debe
* ser >=192). Esto permite que el puntero sea diferenciado de un label, dado que este ultimo debe comenzar 
* con dos ceros (rango 0 a 63).El campo del offset especifica un desplazamiento desde el comienzo del mensaje. 
*
*           +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*           | 1  1|                OFFSET                   |
*           +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/

/*
 * Obtiene un nombre de dominio a partir de la respuesta. 
 * *reader   - Puntero a la posicion del comienzo de un nombre de dominio que se encuentra 
 *             en la seccion ANSWER de la consulta realizada al DNS. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *count    - Puntero que retorna las posiciones que el reader se movio. Depende si el nombre de dominio 
 *             era un puntero (se mueve dos) o un string (se mueve la cantidad de caracteres de dicho string) 
 */

unsigned char* readName(unsigned char *reader, unsigned char *response, int *count)
{
    unsigned char *name = (unsigned char*) malloc(256);
    unsigned int p = 0, jumped = 0, offset;
    *count = 1;
 
    // Valor para obtener el offset de un puntero (si es que hay).
    int bitMask = 49152; // 11000000 00000000
    name[0]='\0';
 
    // Leemos el nombre de dominio con formato (longitud,dato), es decir: 3www6google3com
    while(*reader != 0)
    {
        if(*reader >= 192)
        {
            // El nombre de dominio se encuentra comprimido por lo que el reader tiene un puntero 
            // al valor real. Desde el comienzo del mensaje se lo debe desplazar a la primera ocurrencia
            // del nombre, es decir, response + offset. (luego del if se suma 1)
            offset = (*reader)*256 + *(reader+1) - bitMask; 
            reader = response + offset - 1;
            // Saltamos a otra locacion de memoria por lo que count no se incrementara
            jumped = 1; 
        }
        else
        {
            name[p++] = *reader;
        }
 
        reader = reader + 1;
 
        if(jumped == 0)
        {
            // Si no hemos saltado a otra locacion de memoria entonces podemos incrementar el count 
            *count = *count + 1; 
        }
    }
 
    name[p] = '\0'; // agregamos el caracter terminador
    if(jumped == 1)
    {
        // Numero de veces que en realidad nos movimos en el paquete.
        *count = *count + 1;
    }
 
    changeFromQNameFormatToNormalFormat(name);
    return name;
}

void printResponse(
    struct DNS_HEADER *query_response_header, 
    struct RES_RECORD *answers,
    struct RES_RECORD *authority, 
    struct RES_RECORD *additional)
{
    printAnswers(query_response_header->an_count, answers);
    printAuthority(query_response_header->ns_count, authority);
    printAdditional(query_response_header->ar_count, additional); 
}

/*
 * Imprime en la consola la respuesta de la consulta realizada al DNS.
 * ans_count - Cantidad de repuestas que el DNS proporciono para una determinada 
 *            consulta (en el formato del host).
 * *answers - Puntero al registro que almacena la respuesta del DNS. 
 */

void printAnswers(int ans_count, struct RES_RECORD *answers)
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

void printAuthority(int ns_count, struct RES_RECORD *auth)
{

}

void printAdditional(int ar_count, struct RES_RECORD *addit)
{

}