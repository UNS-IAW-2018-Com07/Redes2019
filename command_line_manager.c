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
    printf("Help");
}

struct argp_option options[] = 
{
    { "hola", 'a', 0, 0, "La consulta se trata de un nombre simbolico y se desea conocer su correspondiente IP numerico asociado" },
    { "help", 'h', 0, 0, 0},
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
    //argp_help(&argp,,0,0);
    printf("Error %i \n", argp_parse(&argp,argc,argv,ARGP_NO_HELP,0,0));
}


unsigned char* getHostname()
{

}

in_addr_t getServer(); 
int getPort(); 
unsigned short getQType(); 
unsigned char getRD(); 
