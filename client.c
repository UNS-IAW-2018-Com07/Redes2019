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
        unsigned char first_dom_name[100];
        unsigned char *serverHostname;
        unsigned char *dom_name = (unsigned char *) malloc(sizeof(unsigned char)*256);
        in_addr_t ip_server;
        
        while(position >= -1)
        {
            printf("\n AFUERA \n");
            qname_length = sendQuery(server, socket_file_descriptor, hostname, 2);//T_NS
            response = receiveQuery(server, socket_file_descriptor);
            getNextServer(response, hostname, qname_length, &dom_name, &ip_server, 0);
            
            strcpy(first_dom_name, dom_name);

            while(strlen(dom_name) > 0 && strcmp(first_dom_name, dom_name)==0)
            {
                printf("\n ADENTRO \n");
                bzero(dom_name,sizeof(dom_name));
                qname_length = sendQuery(server, socket_file_descriptor, first_dom_name, 1);//T_A
                response = receiveQuery(server, socket_file_descriptor);
                getNextServer(response, first_dom_name, qname_length, &dom_name, &ip_server, 1);
                
                if(ip_server != 0) 
                {
                    server.sin_addr.s_addr = ip_server; 
                }
            }

            // Chequeo que no me haya respondido servidor vacio en la consulta anterior
            // Si me respondio con servidor vacio lo mantengo ya que es el autoritativo 
            // al hostname que estoy preguntando. 
            if(ip_server != 0) 
            {
                server.sin_addr.s_addr = ip_server; 
            }

            if(position > -1)
            {
                prepareNextHostname(hostname, position, splited_hostname);
            }
            position--;
        }   

        qname_length = sendQuery(server, socket_file_descriptor, hostname, getQType());
        response = receiveQuery(server, socket_file_descriptor);
        handleResponse(response, qname_length);
        free(dom_name);
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
    if(hostname != NULL ) 
    {
        strcpy(aux, ".");
    }
    strcat(aux, hostname); 
    strcpy(hostname, splited_hostname[position]);
    strcat(hostname, aux);

    // if(hostname[strlen(hostname)-1] == '.')
    // {
    //     hostname[strlen(hostname)-1]='\0'; // Le saco el punto
    // }
}