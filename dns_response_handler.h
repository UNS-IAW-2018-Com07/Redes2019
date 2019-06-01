#include <arpa/inet.h>
#include "dns_response_printer.h"

int handleResponse(unsigned char *response, int qname_length, int show_if_no_answer);

in_addr_t* getNextServer(unsigned char *response, unsigned char* hostname, int qname_length, unsigned char **dom_name);

//unsigned char* getServerHostname(unsigned char *response, int qname_length);

//in_addr_t getNextServer(unsigned char *response, int qname_length);