#include <arpa/inet.h>
#include "dns_response_printer.h"

int handleResponse(unsigned char *response, int qname_length);

unsigned char* getServerHostname(unsigned char *response, int qname_length);

in_addr_t getNextServer(unsigned char *response, int qname_length);