#include <arpa/inet.h>

/*
 * Toma los argumentos del programa, los analiza y almacena.
 * argc - Cantidad de argumentos.
 * *argv - Arreglo que contiene los argumentos del programa.
*/
void setInputValues(int argc, char* argv[]);

/*
 * Retorna el nombre de dominio que se ingresó como argumento.
*/
char* getHostname(); 

/*
 * Retorna el servidor que se ingresó como argumento. 
 * Si no se ingresó un servidor, retorna el servidor DNS contenido en el archivo /etc/resolv.conf
*/
in_addr_t getServer(); 

/*
 * Retorna el puerto que se ingresó como argumento. 
 * Si no se ingresó un puerto, retorna el puerto por defecto 53.
*/
int getPort(); 

/*
 * Retorna el tipo de consulta que se ingresó como argumento. 
 * Si no se ingresó tipo de consulta, retorna tipo A.
*/
unsigned short getQType(); 

/*
 * Retorna la manera en la cual se especificó en los argumentos que se desea realizar la consulta. 
 * Si no se especificó, retorna que se debe realizar de forma recursiva.
*/
unsigned char getRD(); 
