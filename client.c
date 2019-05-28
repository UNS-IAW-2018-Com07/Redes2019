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
#include "command_line_manager.h"
#include "query_manager.h"

in_addr_t getNextServer(unsigned char *response);
int splitHostname(unsigned char **splited_hostname);
void prepareNextHostname(unsigned char *hostname, int position, unsigned char **splited_hostname);

int main( int argc , char *argv[])
{
    setInputValues(argc,argv);
    
    int socket_file_descriptor;
    struct sockaddr_in server;

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = getServer();
    server.sin_port = htons((short) getPort()); 

    // Creacion del socket - devuelve -1 si da error 
    if((socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("Error al intentar abrir el socket. \n");
        exit(errno); 
    }

    if(getRD()) // Bit de recursión activado
    {
        int qname_length = sendQuery(server, socket_file_descriptor, getHostname(), getQType());
        unsigned char *response = receiveQuery(server, socket_file_descriptor);
        handleResponse(response, qname_length);
    }
    else // Hay que imprimir el trace
    {
        unsigned char *splited_hostname[100]; //almacena cada label en una posicion del arreglo
        int position = splitHostname(splited_hostname);
        
        unsigned char hostname[100];
        while(position >= 0)
        {
            printf("%i \n", position);
            //pedir por lo que tiene hostname
            //si no tengo answer, leer el soa de authority, pedir la ip de ese que lei y a ese preguntarle por el hostname
            // mostrar las respuestas que obtuve
            prepareNextHostname(hostname, position, splited_hostname);
            printf("%s \n", hostname);
            position--;
        }

        /*
        position = 0;
        while(splited_hostname[position] != NULL )
        {
            printf("%s \n", splited_hostname[position]);
            position++;
        } */ 

        // int qname_length = sendQuery(server, socket_file_descriptor, ".", getQType());
        // unsigned char *response = receiveQuery(server, socket_file_descriptor);
        // handleResponse(response, qname_length);

        // server.sin_addr.s_addr = getNextServer(response);
        // qname_length = sendQuery(server, socket_file_descriptor, "edu.ar", getQType());
        // response = receiveQuery(server, socket_file_descriptor);
        // handleResponse(response, qname_length);
    }
    
    return 0;
}

in_addr_t getNextServer(unsigned char *response) {
    return inet_addr("8.8.8.8");
}

int splitHostname(unsigned char **splited_hostname)
{
    int position = 0;
    splited_hostname[position] = strtok(getHostname(), ".");
    while(splited_hostname[position] != NULL )
    {
        position++;
        splited_hostname[position] = strtok(NULL, ".");
    } 
    position--;
    return position;
}

void prepareNextHostname(unsigned char *hostname, int position, unsigned char **splited_hostname)
{
    unsigned char aux[100];
    strcpy(aux, hostname); 
    strcpy(hostname, ".");
    if(position == 0) // Evita que quede un punto al principio cuando esta el hostname completo (ejemplo: .cs.uns.edu.ar)
    {
        strcpy(hostname, "");
    }
    strcat(hostname, splited_hostname[position]);
    strcat(hostname, aux);
}

