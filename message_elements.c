#include "message_elements.h"

struct DNS_HEADER
{
    
    unsigned char qr :1;     // identifica si es una consulta (0) o una respuesta (1)
    unsigned char opcode :4; // especifica el tipo de consulta: estandar (0), inversa (1), estado servidor (2)
    unsigned char aa :1;     // se usa en las respuestas y especifica que el dns que otorga la respuesta es autoritativo para el dominio consultado 
    unsigned char tc :1;     // especifica si el mensaje fue truncado por tener un tamanio mayor al permitido
    unsigned char rd :1;     // se setea en la consulta y se copia en la respuesta. Consulta resuelta: iterativamente (0) y recursivamente (1)
    unsigned char ra :1;     // especifica si el servidor puede responder recursivamente
    unsigned char z :1;      // tiene que valer 0. Reservado para uso futuro    
    unsigned char rcode :4;  // Cdigo de respuesta: 
                                // (0) No hubo error. 
                                // (1) El servidor no puedo interpretar la consulta 
                                // (2) El DNS tuvo un problema y no puede procesar la consulta 
                                // (3) El dominio referenciado no existe. Solamente tiene significado cuando responde un DNS autoritativo 
                                // (4) El dns no soporta la consulta requerida
                                // (5) El dns se rehusa a realizar la operacion especificada por razones de politicas
    unsigned short qd_count; // numero de entradas de la seccion QUESTION
    unsigned short an_count; // numero de entradas de la seccion ANSWER
    unsigned short ns_count; // numero de resource records de DNS que estan dentro de la seccion AUTHORITY 
    unsigned short ar_count; // numero de resource records en la seccion ADDITIONAL
    //unsigned char cd :1;     // checking disabled
    //unsigned char ad :1;     // authenticated data  
    unsigned short id;       // se usa para unir las respuestas con las solicitudes  
};

struct QUESTION
{
    unsigned char *qname;   // nombre simbolico que se quiere resolver
    unsigned short qtype;   // tipo del campo del dns que se desea consultar
    unsigned short qclass;  // clase de la consulta: IN (internet)
};

struct RES_RECORD
{
    unsigned char *name;     // nombre simbolico al cual pertenece el resource record
    unsigned short type;     // tipo del campo del dns. Especifica el significado de la informacion que contiene rdata
    unsigned short _class;   // clase de los datos que se encuentran en el campo rdata
    unsigned int ttl;        // tiempo en segundos que el resource record puede ser cacheado antes de ser descartado (tiempo de vida)
    unsigned short data_len; // tamanio en bytes del campo rdata
    unsigned char *rdata;    // describe el recurso. Depende del TYPE y de la CLASS
};