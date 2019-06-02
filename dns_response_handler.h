#include <arpa/inet.h>
#include "dns_response_printer.h"

/*
 * Obtiene los distintos RR de las secciones ANSWER, AUTHORITY y ADDITIONAL a partir de la respuesta a una consulta DNS y los imprime por consola.
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS.
 * qname_length - Longitud del hostname consultado de acuerdo al formato enviado (QName del RFC). 
 */
void handleResponse(unsigned char *response, int qname_length);

/*
 * Calcula el proximo servidor a consultar en la siguiente iteracion.
 * *response - Puntero al comienzo de la respuesta proporcionada por el DNS. 
 * *hostname - string que se le consulto al DNS en formato legible. 
 * qname_length - Longitud del hostname consultado de acuerdo al formato enviado (QName del RFC). 
 * **dom_name - Proximo nombre de dominio que se debe realizar una consulta de tipo NS. 
 * *server - Almacena el proximo servidor a consultar. Vale cero si se debe continuar consultando al actual. 
 * change_dom_name - Establece si se resolvio una consulta NS anteriormente y se esta buscando por un registro de tipo A que resuelva el hostname solicitado en la consulta anterior. Vale 1 si es verdadero, 0 si es falso. 
 */
void getNextServer(unsigned char *response, unsigned char* hostname, int qname_length, unsigned char **dom_name, in_addr_t *server, int change_dom_name);