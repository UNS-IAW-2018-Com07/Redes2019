#include "dns_response_handler.h"
#include "location_reader.h"

int* readAnswers(int ans_count, unsigned char *reader, unsigned char *response, struct RES_RECORD *answers);
void readAuthorities(int ns_count, unsigned char *reader, unsigned char *response, struct RES_RECORD *auth);
void readAdditional(int ar_count, unsigned char *reader, unsigned char *response, struct RES_RECORD *addit);
unsigned char* readName(unsigned char *reader, unsigned char *response, int *count); 

int stop = 0;

void handleResponse(unsigned char *response, int qname_length)
{
    struct RES_RECORD answers[20], auth[20], addit[20];
    int* preferences;

    struct DNS_HEADER *query_response_header = (struct DNS_HEADER *) response;
    unsigned char *reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)];
   
    preferences = readAnswers(ntohs(query_response_header->an_count), reader, response, answers);
    readAuthorities(ntohs(query_response_header->an_count), reader, response, auth);
    readAdditional(ntohs(query_response_header->an_count), reader, response, addit);

    printResponse(query_response_header, preferences, answers, auth, addit);
}

/*
 * Lee las respuestas proporcionadas por el DNS de acuerdo a su tipo: A, MX, LOC, CNAME, entre otros. 
 * ans_count - Cantidad de repuestas que el DNS proporciono para una determinada 
 *             consulta (en el formato del host).
 * *reader   - Puntero a la seccion ANSWER de la consulta realizada al DNS. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *answers  - Puntero al registro que almacenara las respuestas del DNS.
 */
int* readAnswers(int ans_count, unsigned char *reader, unsigned char *response, struct RES_RECORD *answers)
{  
    int* preferences = malloc(sizeof(int)*20);;
 
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

    return preferences; 
}

void readAuthorities(int ns_count, unsigned char *reader, unsigned char *response, struct RES_RECORD *auth)
{
    for(int i = 0; i < ns_count; i++)
    {
        auth[i].name = readName(reader,response,&stop);
        reader += stop;
 /*
        auth[i].resource=(struct R_DATA*)(reader);
        reader+=sizeof(struct R_DATA);
 
        auth[i].rdata=ReadName(reader,buf,&stop);
        reader+=stop;
        */
    }
}

void readAdditional(int ar_count, unsigned char *reader, unsigned char *response, struct RES_RECORD *addit)
{

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
