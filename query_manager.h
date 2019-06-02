#include <sys/socket.h>

/*
 * Envía una consulta DNS. Retorna la longitud total del qname que se envió en la sección QUESTION.
 * server - Servidor al cuál se le consulta.
 * socket_file_descriptor - Socket por el cual se envía la consulta.
 * *hostname - Nombre de dominio por el cual se consulta.
 * qtype - Tipo de RR por el cual se consulta.
*/
int sendQuery(struct sockaddr_in server, int socket_file_descriptor, char *hostname, unsigned short qtype);

/*
 * Recibe una respuesta DNS y la retorna.
 * server - Servidor del cual se espera la respuesta.
 * socket_file_descriptor - Socket por el cual se espera la respuesta.
*/
unsigned char* receiveQuery(struct sockaddr_in server, int socket_file_descriptor);