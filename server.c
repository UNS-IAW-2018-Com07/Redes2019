#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <netdb.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    extern int errno; 
    //extern char *sys_errlist[]; 
    
    const int portnum = 126; 
    int socket_file_descriptor;
    struct sockaddr_in server;
    char buffer[1024];

    bzero(&server, sizeof(server)); 
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons((short) portnum); 
    
    // Creacion del socket - devuelve -1 si da error 
    if((socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("Error al intentar abrir el socket. \n");
        exit(errno); 
    }
    printf("[+]Socket: %i", socket_file_descriptor);
    // Asociar una direccion al socket
    if(bind(socket_file_descriptor, (struct sockaddr*) &server, sizeof(server)))
    {
        perror("Error al intentar asociar el socket a una direccion. \n");
        exit(errno); 
    } 
    // Atender conexiones
    if(recv(socket_file_descriptor, buffer, sizeof(buffer), 0)<0)
    {
        perror("Error al intentar comenzar a atender conexiones. \n");
        exit(errno); 
    }
    printf("[+]Data Received: %s", buffer);
}