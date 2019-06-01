#include "dns_response_handler.h"
#include "location_reader.h"

int* readAnswers(int ans_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *answers);
void readAuthorities(int ns_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *auth);
void readAdditional(int ar_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *addit);
unsigned char* readName(unsigned char *reader, unsigned char *response, int *count); 
void freeVariables(struct DNS_HEADER *query_response_header, int* preferences, struct RES_RECORD *answers, struct RES_RECORD *auth, struct RES_RECORD *addit);
int readLine(unsigned char **reader, unsigned char *response, struct RES_RECORD *rrecord);
int hasAllocated(int qtype);

int isDomainName(unsigned char* dom_name, unsigned char *hostname, int quantity, struct RES_RECORD *rrecords);
unsigned char* getDName(unsigned char *hostname, int quantity, struct RES_RECORD *rrecords);
void getServerIP(in_addr_t *server, unsigned char *dom_name, int quantity, struct RES_RECORD *rrecords);

int stop;

int handleResponse(unsigned char *response, int qname_length, int show_if_no_answer)
{
    int result = EXIT_SUCCESS;

    struct RES_RECORD answers[60], auth[60], addit[60];
    int* preferences;
    unsigned char *reader;

    struct DNS_HEADER *query_response_header = (struct DNS_HEADER *) response;
    reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)];
   
    if(ntohs(query_response_header->an_count)==0)
    {
        result = EXIT_FAILURE;
        if(!show_if_no_answer)
        {
            return result;
        }
    }

    preferences = readAnswers(ntohs(query_response_header->an_count), &reader, response, answers);
    readAuthorities(ntohs(query_response_header->ns_count), &reader, response, auth);
    readAdditional(ntohs(query_response_header->ar_count), &reader, response, addit);

    printResponse(query_response_header, preferences, answers, auth, addit);
    freeVariables(query_response_header, preferences, answers, auth, addit);
    bzero(reader, sizeof(reader));  
    return result;  
}

void freeVariables(struct DNS_HEADER *query_response_header, int* preferences, struct RES_RECORD *answers, struct RES_RECORD *auth, struct RES_RECORD *addit)
{
    for(int i = 0; i < ntohs(query_response_header->an_count); i++)
    {
        free(answers[i].name);
        hasAllocated(ntohs(answers[i].resource_constant->type)) ? free(answers[i].rdata): NULL;
    }
    for(int i = 0; i < ntohs(query_response_header->ns_count); i++)
    {
        free(auth[i].name);
        hasAllocated(ntohs(auth[i].resource_constant->type)) ? free(auth[i].rdata): NULL;
    }
    for(int i = 0; i < ntohs(query_response_header->ar_count); i++)
    {
       free(addit[i].name);
       hasAllocated(ntohs(addit[i].resource_constant->type)) ? free(addit[i].rdata): NULL;
    }
    free(preferences); 
    bzero(answers, sizeof(answers));
    bzero(auth, sizeof(auth));
    bzero(addit, sizeof(addit));
    bzero(query_response_header, sizeof(struct DNS_HEADER));
}

int hasAllocated(int qtype)
{
    switch (qtype)
    {
        case T_A:
        case T_MX:
        case T_CNAME:
        case T_PTR:
        case T_SOA:
        case T_NS:
        {
            return 1; //true;
        }; 
        default:
        {
            return 0; //false;
        }
    }
}

/*
 * Lee las respuestas proporcionadas por el DNS de acuerdo a su tipo: A, MX, LOC, CNAME, entre otros. 
 * ans_count - Cantidad de repuestas que el DNS proporciono para una determinada 
 *             consulta (en el formato del host).
 * *reader   - Puntero a la seccion ANSWER de la consulta realizada al DNS. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *answers  - Puntero al registro que almacenara las respuestas del DNS.
 */
int* readAnswers(int ans_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *answers)
{  
    stop = 0;
    int* preferences = malloc(sizeof(int)*60);
 
    for(int i = 0; i < ans_count; i++)
    {
        preferences[i] = readLine(reader, response, &answers[i]);
    }
    return preferences; 
}

void readAuthorities(int ns_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *auth)
{
    for(int i = 0; i < ns_count; i++)
    {
        readLine(reader, response, &auth[i]);
    }
}

void readAdditional(int ar_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *addit)
{
    for(int i = 0; i < ar_count; i++)
    {
        readLine(reader, response, &addit[i]);
    }
}

int readLine(unsigned char **reader, unsigned char *response, struct RES_RECORD *rrecord)
{
    unsigned char *aux = *reader;
    int preference = 0; 

    (*rrecord).name = readName(aux, response, &stop);
    aux = aux + stop;
 
    (*rrecord).resource_constant = (struct RES_RECORD_CONSTANT*)(aux);
    aux = aux + sizeof(struct RES_RECORD_CONSTANT);

    int type = ntohs((*rrecord).resource_constant->type);
    switch(type)
    {
        case T_A:
        {
            (*rrecord).rdata = (unsigned char*)malloc(ntohs((*rrecord).resource_constant->data_len)+1);
            for(int j = 0; j < ntohs((*rrecord).resource_constant->data_len); j++)
            {
                (*rrecord).rdata[j] = aux[j];
            }
            (*rrecord).rdata[ntohs((*rrecord).resource_constant->data_len)] = '\0';
            aux = aux + ntohs((*rrecord).resource_constant->data_len);
        }; break;
        case T_MX:
        {
            preference = (int)aux[1];
            aux = aux + 2;
            (*rrecord).rdata = readName(aux, response, &stop);
            aux = aux + stop;
        }; break;
        case T_LOC:
        {
            (*rrecord).rdata = (unsigned char*)loc_ntoa(aux, NULL);
            aux = aux + ntohs((*rrecord).resource_constant->data_len);
        }; break;
        case T_CNAME:
        case T_PTR:
        case T_SOA:
        case T_NS:
        {
            (*rrecord).rdata = readName(aux, response, &stop);
            aux = aux + ntohs((*rrecord).resource_constant->data_len);
        }; break;
        default: 
        {
            // Salteamos los RR que no sean los tipos de arriba
            aux = aux + ntohs((*rrecord).resource_constant->data_len);
        }    
    }
    *reader = aux; 
    return preference; 
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

in_addr_t* getNextServer(unsigned char *response, unsigned char* hostname, int qname_length, unsigned char **dom_name)
{
    struct RES_RECORD answers[60], auth[60], addit[60];
    unsigned char *reader,*aux_reader;
    in_addr_t *server = (in_addr_t *) malloc(sizeof(in_addr_t)); 

    struct DNS_HEADER *query_response_header = (struct DNS_HEADER *) response;
    reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)]; 

    readAnswers(ntohs(query_response_header->an_count), &reader, response, answers);
    readAuthorities(ntohs(query_response_header->ns_count), &reader, response, auth);
    readAdditional(ntohs(query_response_header->ar_count), &reader, response, addit);

    // Buscamos por un RR de tipo A que se corresponda con el hostname
    getServerIP(server, hostname, ntohs(query_response_header->an_count), answers);
    if(*server == 0)
    {
        getServerIP(server, hostname, ntohs(query_response_header->ns_count), auth);
    }
    if(*server > 0)
    {
        return server; 
    }

    // Buscamos por un RR de tipo NS/SOA en el answer o authority su correspondiente
    // ip en el additional. 
    for(int i = 0; i < ntohs(query_response_header->ar_count); i++)
    {
        if(isDomainName(addit[i].name, hostname, ntohs(query_response_header->an_count), answers) == 1)
        {
            *server = *((long*)addit[i].rdata); // falta chequear que es de tipo A
            return server; 
        }
        
        if(isDomainName(addit[i].name, hostname, ntohs(query_response_header->ns_count), auth) == 1)
        {
            *server = *((long*)addit[i].rdata); // falta chequear que es de tipo A
            return server; 
        }
    }

    // Si no encontro un RR additional tal que mapee a un answer o authority entonces
    // buscamos el nombre de dominio de dicho hostname. (SOA/NS)
    (*dom_name) = getDName(hostname, ntohs(query_response_header->an_count), answers); 
    if(strlen(*dom_name)==0)
    {
        (*dom_name) = getDName(hostname, ntohs(query_response_header->ns_count), auth); 
    }
    // Hay que liberar rrecords y dom_name. 
    
    //printResponse(query_response_header, preferences, answers, auth, addit);
    //freeVariables(query_response_header, preferences, answers, auth, addit);
    bzero(reader, sizeof(reader));  
    return server;  
}

int isDomainName(unsigned char* dom_name, unsigned char *hostname, int quantity, struct RES_RECORD *rrecords)
{
    for(int i = 0; i < quantity; i++)
    {
        if(strcmp(rrecords[i].name, hostname) == 0) 
        {
            if(ntohs(rrecords[i].resource_constant->type) == T_SOA ||
               ntohs(rrecords[i].resource_constant->type) == T_NS)
            {
                if(strcmp(dom_name, rrecords[i].rdata))
                {
                    return 1; 
                }
            }
        }
    }
    return 0; 
}

unsigned char* getDName(unsigned char *hostname, int quantity, struct RES_RECORD *rrecords)
{
    unsigned char* dom_name = (unsigned char*) malloc(sizeof(char)*256);
    for(int i = 0; i < quantity; i++)
    {
        if(strcmp(rrecords[i].name, hostname) == 0) 
        {
            if(ntohs(rrecords[i].resource_constant->type) == T_SOA ||
               ntohs(rrecords[i].resource_constant->type) == T_NS)
            {
                strcpy(dom_name, rrecords[i].rdata);
                return dom_name; 
            }
        }
    }
    return dom_name; 
}

/*
in_addr_t getNextServer(unsigned char *response, unsigned char* soa_name, int qname_length)
{
    struct RES_RECORD rrecords[60];
    unsigned char *reader;
    in_addr_t server; 

    struct DNS_HEADER *query_response_header = (struct DNS_HEADER *) response;
    reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)]; 
    server = getA(soa_name, ntohs(query_response_header->an_count), &reader, response, rrecords);
    //if(strlen(server) == 0) 
    //{
    //    bzero(rrecords, sizeof(rrecords)); /// Hay que liberar
    //    getSOA(hostname, ntohs(query_response_header->ns_count), &reader, response, rrecords);
    //}
    // Hay que liberar rrecords. 
    
    //printResponse(query_response_header, preferences, answers, auth, addit);
    //freeVariables(query_response_header, preferences, answers, auth, addit);
    bzero(reader, sizeof(reader));  
    return server;  
}
*/

void getServerIP(in_addr_t* server, unsigned char *hostname, int quantity, struct RES_RECORD *rrecords)
{
    long *ipv4 = 0;
    for(int i = 0; i < quantity; i++)
    {
        if(strcmp(rrecords[i].name, hostname) == 0) 
        {
            if(ntohs(rrecords[i].resource_constant->type) == T_A)
            {
                ipv4 = (long*)rrecords[i].rdata;
                *server = *ipv4;
            }
        }
    }
}

/*
unsigned char* getServerHostname(unsigned char *response, int qname_length)
{
    unsigned char *reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)];
    readName(reader, response, &stop); //me salteo el nombre
    reader += stop + sizeof(struct RES_RECORD_CONSTANT);
    return readName(reader, response, &stop);
}
*/

// in_addr_t getNextServer(unsigned char *response, int qname_length) 
// {
//     unsigned char *reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)];
//     struct RES_RECORD answer[1];
//     answer[1].name = readName(reader, response, &stop);
//     reader = reader + stop;
//     answer[1].resource_constant = (struct RES_RECORD_CONSTANT*)reader;
//     reader = reader + sizeof(struct RES_RECORD_CONSTANT);
//     answer[1].rdata = (unsigned char*)malloc(ntohs(answer[1].resource_constant->data_len) + 1);
//     for(int j = 0; j < ntohs(answer[1].resource_constant->data_len); j++)
//     {
//         answer[1].rdata[j]=reader[j];
//     }
//     answer[1].rdata[ntohs(answer[1].resource_constant->data_len)] = '\0';
//     printf("\n SERVIDOR %s\n", answer[1].rdata);
//     in_addr_t result = inet_addr(answer[1].rdata);
//     free(answer[1].rdata);
//     return result;
// }

/*
in_addr_t getNextServer(unsigned char *response, int qname_length) 
{
    unsigned char *reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)];
    struct RES_RECORD answer[1]; 
    int stop_aux = 0;
    unsigned char *aux = reader; 
    readName(aux, response, &stop_aux);
    aux = aux + stop_aux;
    answer[1].resource_constant = (struct RES_RECORD_CONSTANT*)(aux);
    aux = aux + sizeof(struct RES_RECORD_CONSTANT);
    answer[1].rdata = (unsigned char*)malloc(ntohs(answer[1].resource_constant->data_len)+1);
    for(int j = 0; j < ntohs(answer[1].resource_constant->data_len); j++)
    {
        answer[1].rdata[j] = aux[j];
    }
    answer[1].rdata[ntohs(answer[1].resource_constant->data_len)] = '\0';
    long *ipv4 = (long*)answer[1].rdata;
    struct sockaddr_in aux_server;
    aux_server.sin_addr.s_addr = (*ipv4);
    printf("SERVIDOR %s\n", inet_ntoa(aux_server.sin_addr));
    in_addr_t result = inet_addr(inet_ntoa(aux_server.sin_addr));
    bzero(reader, sizeof(reader));
    bzero(aux, sizeof(aux));
    bzero(answer, sizeof(answer));
    return result;
}
*/