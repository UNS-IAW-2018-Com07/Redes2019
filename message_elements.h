#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct DNS_HEADER
{
    unsigned short id;       // se usa para unir las respuestas con las solicitudes  

    unsigned char rd :1;     // se setea en la consulta y se copia en la respuesta. Consulta resuelta: iterativamente (0) y recursivamente (1)
    unsigned char tc :1;     // especifica si el mensaje fue truncado por tener un tamanio mayor al permitido
    unsigned char aa :1;     // se usa en las respuestas y especifica que el dns que otorga la respuesta es autoritativo para el dominio consultado 
    unsigned char opcode :4; // especifica el tipo de consulta: estandar (0), inversa (1), estado servidor (2)
    unsigned char qr :1;     // identifica si es una consulta (0) o una respuesta (1)

    unsigned char rcode :4;  /**
                             * Codigo de respuesta:
                             *  (0) No hubo error. 
                             *  (1) El servidor no pudo interpretar la consulta 
                             *  (2) El DNS tuvo un problema y no puede procesar la consulta 
                             *  (3) El dominio referenciado no existe. Solamente tiene significado cuando responde un DNS autoritativo 
                             *  (4) El DNS no soporta la consulta requerida
                             *  (5) El DNS se rehusa a realizar la operacion especificada por razones de politicas
                            */        
    unsigned char z :3;      // tiene que valer 0. Reservado para uso futuro
    unsigned char ra :1;     // especifica si el servidor puede responder recursivamente
    
    unsigned short qd_count; // numero de entradas de la seccion QUESTION
    unsigned short an_count; // numero de entradas de la seccion ANSWER
    unsigned short ns_count; // numero de resource records de DNS que estan dentro de la seccion AUTHORITY 
    unsigned short ar_count; // numero de resource records en la seccion ADDITIONAL    
};

struct QUESTION_CONSTANT
{
    unsigned short qtype;   // tipo del campo del dns que se desea consultar
    unsigned short qclass;  // clase de la consulta: IN (internet)
};

#pragma pack(push, 1) 
// Apila en la pila interna el valor anterior para alinear los campos de las estructuras.
// A continuación ajusta el nuevo valor para alinear los campos.
struct RES_RECORD_CONSTANT
{
    unsigned short type;   /* tipo del campo del dns. Especifica el significado de la informacion que contiene rdata
                            * A = 1 
                            * LOC = 29
                            * MX = 15
                            * */

    unsigned short _class;   // clase de los datos que se encuentran en el campo rdata
    unsigned int ttl;        // tiempo en segundos que el resource record puede ser cacheado antes de ser descartado (tiempo de vida)
    unsigned short data_len; // tamanio en bytes del campo rdata
};
#pragma pack(pop)
// Ajusta el valor para alinear los campos de las estructuras al último que se utilizó.

struct RES_RECORD
{
    unsigned char *name;     // nombre simbolico al cual pertenece el resource record
    struct RES_RECORD_CONSTANT *resource_constant; 
    unsigned char *rdata;    // describe el recurso. Depende del TYPE y de la CLASS
};

/*
 * El nombre por el cual se consulta esta dividido en secciones segun los puntos. 
 * Cada seccion se representa como un par (longitud, dato). 
 * La longitud es un entero de un byte que representa la cantidad de caracteres de un dato. 
 * Convierte por ejemplo: www.uns.edu.ar a 3www3uns3edu2ar 
 * 
 * *qname    - Almacena la respuesta. Posee los pares (longitud, dato) del nombre por 
 *             el cual se consulta. 
 *             Siguiendo el ejemplo anterior, tomaria el valor 3www3uns3edu2ar
 * *hostname - Nombre por el cual se desea obtener el formato correspondiente al QName.
 *             Siguiendo el ejemplo anterior, seria www.uns.edu.ar
 */
void changeToQNameFormat(unsigned char* qname, char* hostname);

/*
 * Inversa al metodo anterior. 
 * Convierte por ejemplo: 3www3uns3edu2ar a www.uns.edu.ar 
 * *qname - Posee los pares (longitud, dato) del nombre por el cual se consulta. 
 *          El metodo sobreescribe el valor de qname para que se lo transforme a 
 *          una cadena de caracteres tal que a cada seccion se la separa por un punto.
 *          Es decir, en un principio tendra 3www3uns3edu2ar y, al finalizar el metodo
 *          su valor almacenara www.uns.edu.ar
 */
void changeFromQNameFormatToNormalFormat(unsigned char* qname);