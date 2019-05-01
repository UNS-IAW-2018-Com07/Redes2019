#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    extern int errno; 
    //extern char *sys_errlist[]; 
    
    const int portnum = 126; 
    int socket_file_descriptor;
    struct sockaddr_in server;
    struct hostent *server_host; 
    char buffer[1024];

    bzero(&server, sizeof(server)); 
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons((short) portnum); 
    
    // Creacion del socket - devuelve -1 si da error 
    if((socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("Error al intentar abrir el socket. \n");
        exit(errno); 
    }
    
    strcpy(buffer, "Hello Server\n");
    if((sendto(socket_file_descriptor, buffer, sizeof(buffer), 0, (struct sockaddr*)&server, sizeof(server)))<sizeof(buffer))
    {
        perror("Error al intentar enviar un mensaje al servidor. \n");
        exit(errno); 
    }
    printf("Data Sent");
}