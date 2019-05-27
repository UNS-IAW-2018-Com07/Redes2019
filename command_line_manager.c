#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <argp.h>
#include "command_line_manager.h"

char* hostname;
in_addr_t server;
int port = 53; 
unsigned short qtype = 1;
unsigned char rd = 1;

unsigned short qtype_entered = 0;
unsigned short rd_entered = 0;

in_addr_t getDefaultDNSServer();

void showUsage()
{
    printf("\nUsage: query consulta @servidor[:puerto] [-a | -mx | -loc] [-r | -t] [-h] \n\n");
}

void showHelp()
{
    showUsage();
    printf("  :puerto     \n");
    printf("  -a          La consulta se trata de un nombre simbólico y se desea conocer su correspondiente IP numerico asociado. \n");
    printf("  --mx        Determina el servidor a cargo de la recepción de correo electrónico para el dominio indicado en la consulta. \n");
    printf("  --loc       Información relativa a la ubicación geográfica del dominio indicado en la consulta. \n");
    printf("  -r          La consulta se resuelve recursivamente. \n");
    printf("  -t          La consulta se resuelve iterativamente, mostrando una traza con la evolución de la misma. \n");
    printf("  -h          Muestra la ayuda. \n\n");
}

void showWrongOptionsError()
{
    printf("\nError: Mal uso de las opciones.");
    showUsage();
}

void showError(char *error_message)
{
    printf("\nError: %s", error_message);
    showUsage();
}

struct argp_option options[] = 
{
    { 0, 'a', 0, 0, 0 },
    { "mx", 'm', 0, 0, 0 },
    { "loc", 'l', 0, 0, 0 },
    { 0, 'r', 0, 0, 0 },
    { 0, 't', 0, 0, 0 },
    { "help", 'h', 0, 0, 0 },
    { 0 }
};

int parse_opt(int key, char *arg, struct argp_state *state)
{
    switch(key)
    {
        case 'a': 
        {
            if (qtype_entered)
            {
                showWrongOptionsError();
                exit(EXIT_FAILURE);
            }
            qtype_entered = 1;
            qtype = 1;
        }
        break;
        case 'm':
        {
            if (qtype_entered)
            {
                showWrongOptionsError();
                exit(EXIT_FAILURE);
            }
            qtype_entered = 1;
            qtype = 15; 
        };
        break;
        case 'l':         
        {
            if (qtype_entered)
            {
                showWrongOptionsError();
                exit(EXIT_FAILURE);
            }
            qtype_entered = 1;
            qtype = 29; 
        };
        break;
        case 'r':         
        {
            if (rd_entered)
            {
                showWrongOptionsError();
                exit(EXIT_FAILURE);
            }
            rd_entered = 1;
            rd = 1; 
        };
        break;
        case 't':         
        {
            if (rd_entered)
            {
                showWrongOptionsError();
                exit(EXIT_FAILURE);
            }
            rd_entered = 1;
            rd = 0; 
        };
        break;
        case 'h': 
        { 
            showHelp(); 
            exit(EXIT_SUCCESS); 
        };
        break;
        case ARGP_KEY_ARG:
        {
            if(arg[0] == '@')
            {
                if(server != 0)
                {
                    showWrongOptionsError();
                    exit(EXIT_FAILURE);
                }

                arg++;

                if(strlen(arg) == 0) //estaba el @ solito
                {
                    showError("El servidor no puede ser vacío.");
                    exit(EXIT_FAILURE);
                }

                if(arg[0] == ':' )
                {
                    showError("Para especificar un puerto se debe ingresar un servidor.");
                    exit(EXIT_FAILURE);
                }

                server = inet_addr(strtok(arg, ":"));

                if(server == (in_addr_t)(-1))
                {
                    showError("DIrección IP del servidor inválida.");
                    exit(EXIT_FAILURE);
                }

                char* port_string = strtok(NULL, "");
                if(port_string != NULL)
                {
                    port = atoi(port_string);
                }
            }
            else 
            {
                if(hostname != NULL)
                {
                    showWrongOptionsError();
                    exit(EXIT_FAILURE);
                }
                hostname = arg; 
            }
        }
        break; 
    }
    return 0; 
}

struct argp argp = { options, parse_opt, 0, 0};

void setInputValues(int argc, char* argv[])
{
    error_t error = argp_parse(&argp, argc, argv, ARGP_NO_HELP, 0, 0);
    if(error)
    {
        showWrongOptionsError();
        exit(error);
    }

    if(hostname == NULL)
    {
        showError("La consulta es un campo obligatorio.");
        exit(EXIT_FAILURE); 
    }  

    if(server == 0)
    {
        server = getDefaultDNSServer();
    }
}

char* getHostname()
{
    return hostname; 
}

in_addr_t getServer()
{
    return server;
} 

int getPort() 
{
    return port;
}

unsigned short getQType()
{
    return qtype;
}

unsigned char getRD()
{
    return rd;
}

/*
 * Obtiene el servidor dns del archivo /etc/resolv.conf
 * */
in_addr_t getDefaultDNSServer()
{
    FILE *nameservers_file;
    char readed_line[200], *query_address;

    if((nameservers_file = fopen("/etc/resolv.conf", "r")) != NULL)
    {
        while(fgets(readed_line, 200 ,nameservers_file))
        {
            if(readed_line[0] != '#')
            {
                if(strncmp(readed_line, "nameserver", 10) == 0)
                {
                    query_address = strtok(readed_line, " ");
                    query_address = strtok(NULL, " ");
                    return inet_addr(query_address);
                }
            }
        }
    }
    showError("No se especificó un servidor y no se tiene uno por defecto");
    exit(EXIT_FAILURE);
}