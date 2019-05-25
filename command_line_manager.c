#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <argp.h>
#include "command_line_manager.h"

unsigned char* hostname;
in_addr_t server;
int port; 
unsigned short qtype;
unsigned char rd; 

void showHelp()
{
    printf("\nUsage: query consulta @servidor[:puerto] [-a | -mx | -loc] [-r | -t] [-h] \n\n");
    printf("  :puerto     \n");
    printf("  -a          La consulta se trata de un nombre simbolico y se \n");
    printf("              desea conocer su correspondiente IP numerico asociado. \n");
    printf("  -mx         \n");
    printf("  -loc        \n");
    printf("  -r          \n");
    printf("  -t          \n");
    printf("  -h          \n\n");
}

struct argp_option options[] = 
{
    { 0, 'a', 0, 0, 0 },
    { "help", 'h', 0, 0, 0 },
    { 0 }
};

int parse_opt(int key, char *arg, struct argp_state *state)
{
    switch(key)
    {
        case 'a': printf("Valor: %s \n", arg); break ;
        case 'h': { showHelp(); exit(0); }; break;
        case ARGP_KEY_ARG:
        {
            if(arg[0] == '@')
            {
                arg++;
                printf("Servidor: %s \n", arg);
            }
            else 
            {
                printf("Consulta: %s \n", arg); 
            }
        }
        break; 
    }
    return 0; 
}

struct argp argp = { options, parse_opt, "consulta @servidor", "HELP NUEVO"};

extern void argp_usage (const struct argp_state *__state){
    printf("print de argp_usage"); 
}

void setInputValues(int argc, char* argv[])
{
    printf("Error %i \n", argp_parse(&argp,argc,argv,ARGP_NO_HELP,0,0));
}


unsigned char* getHostname()
{
    return hostname; 
}

in_addr_t getServer(); 
int getPort(); 
unsigned short getQType(); 
unsigned char getRD(); 
