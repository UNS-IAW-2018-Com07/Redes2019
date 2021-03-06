#include "dns_response_handler.h"
#include "location_reader.h"

int* readAnswers(int ans_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *answers);
void readAuthorities(int ns_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *auth);
void readAdditional(int ar_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *addit);
unsigned char* readName(unsigned char *reader, unsigned char *response, int *count); 
void freeVariables(struct DNS_HEADER *query_response_header, int* preferences, struct RES_RECORD *answers, struct RES_RECORD *auth, struct RES_RECORD *addit);
int readLine(unsigned char **reader, unsigned char *response, struct RES_RECORD *rrecord);
int hasAllocated(int type);
int isDomainName(unsigned char* dom_name, int quantity, struct RES_RECORD *rrecords);
void getDName(unsigned char **dom_name, int quantity, struct RES_RECORD *rrecords);
void getServerIP(in_addr_t *server, unsigned char *dom_name, int quantity, struct RES_RECORD *rrecords);

int stop;

void handleResponse(unsigned char *response, int qname_length)
{
    int result = EXIT_SUCCESS;

    struct RES_RECORD answers[60], auth[60], addit[60];
    int* preferences;
    unsigned char *reader;

    struct DNS_HEADER *query_response_header = (struct DNS_HEADER *) response;
    reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)];

    preferences = readAnswers(ntohs(query_response_header->an_count), &reader, response, answers);
    readAuthorities(ntohs(query_response_header->ns_count), &reader, response, auth);
    readAdditional(ntohs(query_response_header->ar_count), &reader, response, addit);

    printResponse(query_response_header, preferences, answers, auth, addit);
    freeVariables(query_response_header, preferences, answers, auth, addit);
    bzero(reader, sizeof(reader));  
}

/*
 * Realiza un free de las variables si es necesario y las reinicia en cero. 
 * *query_response_header - Puntero al encabezado de la consulta realizada al DNS. 
 * *preferences - Puntero a los valores de las distintas preferencias de los RR de tipo MX para la seccion ANSWER. 
 * *answers - Puntero a los registros que almacenan las respuestas de la seccion ANSWER.
 * *auth - Puntero a los registros que almacenan las respuestas de la seccion AUTHORITY.
 * *addit - Puntero a los registros que almacenan las respuestas de la seccion ADDITIONAL.
 */
void freeVariables(struct DNS_HEADER *query_response_header, int *preferences, struct RES_RECORD *answers, struct RES_RECORD *auth, struct RES_RECORD *addit)
{
	int i;
    for(i = 0; i < ntohs(query_response_header->an_count); i++)
    {
        free(answers[i].name);
        hasAllocated(ntohs(answers[i].resource_constant->type)) ? free(answers[i].rdata): NULL;
    }
    for(i = 0; i < ntohs(query_response_header->ns_count); i++)
    {
        free(auth[i].name);
        hasAllocated(ntohs(auth[i].resource_constant->type)) ? free(auth[i].rdata): NULL;
    }
    for(i = 0; i < ntohs(query_response_header->ar_count); i++)
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

/*
 * Determina si para el tipo de RR dado se debio realizar malloc.
 * Retorna 1 si se debio realizar malloc y retorna 0 cuando no.  
 * type - Tipo del RR.
 */
int hasAllocated(int type)
{
    switch (type)
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
 * Lee los RR de la seccion ANSWER proporcionada por el DNS de acuerdo a su tipo: A, MX, LOC, CNAME, entre otros. 
 * Retorna un arreglo de enteros que representa la preferencia de cada una de las consultas MX. Si el RR no era de tipo MX entonces en la posicion se almacenara un cero. 
 * ans_count - Cantidad de repuestas de la seccion ANSWER.
 * **reader - Puntero a un puntero que apunta al comienzo de la seccion ANSWER. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *answers - Puntero al registro que almacenara las respuestas de la seccion ANSWER.
 */
int* readAnswers(int ans_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *answers)
{  
    stop = 0;
    int* preferences = malloc(sizeof(int)*60);
	int i;
	
    for(i = 0; i < ans_count; i++)
    {
        preferences[i] = readLine(reader, response, &answers[i]);
    }
    return preferences; 
}

/*
 * Lee las respuestas proporcionadas por el DNS de acuerdo a su tipo: A, MX, LOC, CNAME, entre otros. 
 * ns_count - Cantidad de repuestas de la seccion AUTHORITY.
 * **reader - Puntero a un puntero que apunta al comienzo de la seccion AUTHORITY. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *auth - Puntero al registro que almacenara las respuestas de la seccion AUTHORITY.
 */
void readAuthorities(int ns_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *auth)
{
	int i; 
    for(i = 0; i < ns_count; i++)
    {
        readLine(reader, response, &auth[i]);
    }
}

/*
 * Lee las respuestas proporcionadas por el DNS de acuerdo a su tipo: A, MX, LOC, CNAME, entre otros. 
 * ar_count - Cantidad de repuestas de la seccion ADDITIONAL.
 * **reader - Puntero a un puntero que apunta al comienzo de la seccion ADDITIONAL. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *addit - Puntero al registro que almacenara las respuestas de la seccion ADDITIONAL.
 */
void readAdditional(int ar_count, unsigned char **reader, unsigned char *response, struct RES_RECORD *addit)
{
	int i;
    for(i = 0; i < ar_count; i++)
    {
        readLine(reader, response, &addit[i]);
    }
}

/*
 * Lee un RR a partir de la respuesta y un puntero al comienzo de la linea que se desea leer. 
 * Retorna un entero que se corresponde con el campo preference del RR de tipo MX. Si el rrecord leido es de otro tipo, el metodo retorna cero.  
 * **reader - Puntero a un puntero que apunta a la posicion de comienzo de un RR de la respuesta. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *rrecord - Retorna el RR leido de la respuesta.  
 */
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
            int j;
            for(j = 0; j < ntohs((*rrecord).resource_constant->data_len); j++)
            {
                (*rrecord).rdata[j] = aux[j];
            }
            (*rrecord).rdata[ntohs((*rrecord).resource_constant->data_len)] = '\0';
            aux = aux + ntohs((*rrecord).resource_constant->data_len);
        }; break;
        case T_MX:
        {
            preference = (int)(aux[0]*256 + aux[1]);
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
 * Retorna un nombre de dominio a partir de la respuesta. 
 * *reader - Puntero a la posicion del comienzo de un nombre de dominio que se encuentra en la seccion ANSWER de la consulta realizada al DNS. 
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.  
 * *count - Puntero que retorna las posiciones que el reader se movio. Depende si el nombre de dominio era un puntero (se mueve dos) o un string (se mueve la cantidad de caracteres de dicho string) 
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

void getNextServer(unsigned char *response, unsigned char* hostname, int qname_length, unsigned char **dom_name, in_addr_t *server, int change_dom_name)
{
    struct RES_RECORD answers[60], auth[60], addit[60];
    unsigned char *reader,*aux_reader;
    int* preferences; 
    *server = 0;

    struct DNS_HEADER *query_response_header = (struct DNS_HEADER *) response;
    reader = &response[sizeof(struct DNS_HEADER) + qname_length + sizeof(struct QUESTION_CONSTANT)]; 

    preferences = readAnswers(ntohs(query_response_header->an_count), &reader, response, answers);
    readAuthorities(ntohs(query_response_header->ns_count), &reader, response, auth);
    readAdditional(ntohs(query_response_header->ar_count), &reader, response, addit);

    // Buscamos por un RR de tipo A que se corresponda con el hostname
    // Si existe entonces retornamos su ip para que sea el proximo server. 
    getServerIP(server, hostname, ntohs(query_response_header->an_count), answers);
    if(*server == 0)
    {
        getServerIP(server, hostname, ntohs(query_response_header->ns_count), auth);
        if(*server == 0)
        {
            getServerIP(server, hostname, ntohs(query_response_header->ar_count), addit);
        }
    }

    // Buscamos por un RR de tipo NS/SOA en el answer o authority su correspondiente
    // ip en el additional (si es que no encontro un RR de tipo A). 
    int i;
    for(i = 0; i < ntohs(query_response_header->ar_count) && *server == 0; i++)
    {
        if(isDomainName(addit[i].name, ntohs(query_response_header->an_count), answers) == 1)
        {
            if(ntohs(addit[i].resource_constant->type) == T_A){
                memcpy(server,((long*)addit[i].rdata),sizeof(in_addr_t));
                if(change_dom_name)
                    memcpy(*dom_name,hostname,sizeof(*dom_name)); //si pregunto afuera del while esto no se deberia mandar
            }
        }
        
        if(isDomainName(addit[i].name, ntohs(query_response_header->ns_count), auth) == 1)
        {
            if(ntohs(addit[i].resource_constant->type) == T_A){
                memcpy(server,((long*)addit[i].rdata),sizeof(in_addr_t));
                if(change_dom_name)
                    memcpy(*dom_name,hostname,sizeof(*dom_name));  
            } 
        }
    }

    // Si no encontro un RR additional tal que mapee a un answer o authority entonces
    // buscamos el nombre de dominio. (SOA/NS)
    if(*server == 0)
    {
        getDName(dom_name, ntohs(query_response_header->an_count), answers); 
        if(strlen(*dom_name)==0)
        {
            getDName(dom_name, ntohs(query_response_header->ns_count), auth); 
        }
    } 
    
    printResponse(query_response_header, preferences, answers, auth, addit);
    freeVariables(query_response_header, preferences, answers, auth, addit);
    bzero(reader, sizeof(reader));  
}

/* 
 * Verifica si un nombre de dominio (dom_name) es autoritativo para algun otro nombre de dominio. 
 * Retorna 1 si es autoritativo y, caso contrario retorna 0.   
 * *dom_name - Puntero que almacena el nombre de dominio a consultar (si es autoritativo o no).  
 * quantity - Cantidad de RR que se encuentran almacenados en *rrecords. 
 * *records - Conjunto de RR en donde se buscara si el nombre de dominio es autoritativo. 
 */
int isDomainName(unsigned char* dom_name, int quantity, struct RES_RECORD *rrecords)
{
	int i; 
    for(i = 0; i < quantity; i++)
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
    return 0; 
}

/* 
 * Obtiene el primer nombre de dominio autoritativo (RR de tipo NS o SOA) que se encuentre en *rrecords.   
 * **dom_name - Puntero que retorna la respuesta. Si no existen RR de tipo NS o SOA entonces la variable se mantiene con su valor original.
 * quantity - Cantidad de RR que se encuentran almacenados en *rrecords. 
 * *records - Conjunto de RR en donde se buscara el nombre de dominio autoritativo. 
 */
void getDName(unsigned char **dom_name, int quantity, struct RES_RECORD *rrecords)
{
	int i;
    for(i = 0; i < quantity; i++)
    {
        if(ntohs(rrecords[i].resource_constant->type) == T_SOA ||
            ntohs(rrecords[i].resource_constant->type) == T_NS)
        {
            strcpy(*dom_name, rrecords[i].rdata);
            return; 
        }
    } 
}

/* 
 * Para un determinado hostname, recorre los RR proporcionados y obtiene el valor de su IP (si existe). 
 * *server - Puntero que retorna la respuesta. Si existe una IP que se corresponde con el hostname se la almacena en formato in_addr_t. Caso contrario, retorna una IP vacia (0.0.0.0). 
 * *hostname - Cadena de caracteres por la cual se desea obtener la IP. 
 * quantity - Cantidad de RR que se encuentran almacenados en *rrecords. 
 * *records - Conjunto de RR en donde se buscara la IP. 
 */
void getServerIP(in_addr_t* server, unsigned char *hostname, int quantity, struct RES_RECORD *rrecords)
{
    *server = 0; 
    long *ipv4 = 0;
    int i; 
    for(i = 0; i < quantity; i++)
    {
        if(strcmp(rrecords[i].name, hostname) == 0) 
        {
            if(ntohs(rrecords[i].resource_constant->type) == T_A)
            {
                memcpy(server,((long*)rrecords[i].rdata),sizeof(in_addr_t)); 
                return; 
            }
        }
    }
}
