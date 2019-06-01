#include <arpa/inet.h>
#include "dns_response_printer.h"

void handleResponse(unsigned char *response, int qname_length);

void getNextServer(unsigned char *response, unsigned char* hostname, int qname_length, unsigned char **dom_name, in_addr_t *server);