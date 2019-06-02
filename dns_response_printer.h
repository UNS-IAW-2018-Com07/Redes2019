#include "message_elements.h"

/* 
 * Imprime por consola las distintas secciones de la respuesta otorgada por el DNS.
 * *query_response_header - Puntero al encabezado de la respuesta.
 * *preferences - Puntero a an_count enteros que almacenan el valor correspondiente al campo preference de una respuesta MX de la seccion ANSWER.
 * *answers - Puntero a los registros que almacenan las respuestas de la seccion ANSWER.
 * *auth - Puntero a los registros que almacenan las respuestas de la seccion AUTHORITY.
 * *addit - Puntero a los registros que almacenan las respuestas de la seccion ADDITIONAL.
 */
void printResponse(
    struct DNS_HEADER *query_response_header, 
    int* preferences,
    struct RES_RECORD *answers,
    struct RES_RECORD *auth, 
    struct RES_RECORD *addit);
