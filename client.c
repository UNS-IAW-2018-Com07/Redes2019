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
#include<unistd.h>

/*
 * Get the DNS servers from /etc/resolv.conf file on Linux
 * */
in_addr_t get_dns_server()
{
    FILE *nameservers_file;
    char readed_line[200], *dns_address;

    if((nameservers_file = fopen("/etc/resolv.conf", "r")) == NULL)
    {
        printf("ACA DEBERIAMOS DECIR QUE NO SE PUDO REALIZAR LA CONSULTA PRQUE NO HAY DNS EN LOS PARAMS NI SABEMOS EL DEFAULT  \n");
    }
     
    while(fgets(readed_line, 200 ,nameservers_file))
    {
        if(readed_line[0] != '#')
        {
            if(strncmp(readed_line, "nameserver", 10) == 0)
            {
                dns_address = strtok(readed_line, " ");
                dns_address = strtok(NULL, " ");
                return inet_addr(dns_address);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    extern int errno; 
    
    const int portnum = 53; 
    int socket_file_descriptor;
    struct sockaddr_in server;
    struct hostent *server_host; 
    char buffer[512];

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = get_dns_server();
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