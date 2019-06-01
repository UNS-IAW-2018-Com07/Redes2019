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

    unsigned char *response;
    int qname_length;

    if(getRD()) // Bit de recursión activado
    {
        qname_length = sendQuery(server, socket_file_descriptor, getHostname(), getQType());
        response = receiveQuery(server, socket_file_descriptor);
        handleResponse(response, qname_length);
    }
    else // Hay que imprimir el trace
    {
        unsigned char *splited_hostname[100]; //almacena cada label en una posicion del arreglo
        int position = splitHostname(splited_hostname);

        unsigned char hostname[100];
        unsigned char *serverHostname;
        unsigned char *dom_name;
        in_addr_t* server_aux;
        
        while(position >= 0)
        {
            qname_length = sendQuery(server, socket_file_descriptor, hostname, 2);//T_NS
            response = receiveQuery(server, socket_file_descriptor);
            server_aux = getNextServer(response, hostname, qname_length, &dom_name);
            
            if(strlen(dom_name) > 0)
            {
                printf("entro a buscar por A del dom_name %s\n",dom_name);
                qname_length = sendQuery(server, socket_file_descriptor, dom_name, 1);//T_A
                response = receiveQuery(server, socket_file_descriptor);
                server_aux = getNextServer(response, hostname, qname_length, &dom_name);
            }

            server.sin_addr.s_addr = *server_aux; 
            printf("%d\n", server.sin_addr.s_addr);
            printf("%s\n", inet_ntoa(server.sin_addr));
            //handleResponse(response, qname_length, 1);
            // if(handleResponse(response, qname_length, 1)==EXIT_FAILURE) // no pudo manejar la respuesta porque no habia answer (es decir, hay que pedir a otro server)
            // {                
            //     // serverHostname = getServerHostname(response, qname_length);
                
            //     // qname_length = sendQuery(server, socket_file_descriptor, serverHostname, getQType());
            //     // response = receiveQuery(server, socket_file_descriptor);

            //     // server.sin_addr.s_addr = getNextServer(response, qname_length);
            // }
            prepareNextHostname(hostname, position, splited_hostname);
            if(hostname[strlen(hostname)-1] == '.')
            {
                hostname[strlen(hostname)-1]='\0'; // Le saco el punto
            }
            position--;
        }
        //qname_length = sendQuery(server, socket_file_descriptor, hostname, getQType());
        //response = receiveQuery(server, socket_file_descriptor);
        //handleResponse(response, qname_length, 1);
    }
    
    return 0;
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
    strcpy(aux, ".");
    strcat(aux, hostname); 
    strcpy(hostname, splited_hostname[position]);
    strcat(hostname, aux);
}