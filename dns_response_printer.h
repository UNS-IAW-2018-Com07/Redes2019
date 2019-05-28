#include "message_elements.h"

void printResponse(
    struct DNS_HEADER *query_response_header, 
    int* preferences,
    struct RES_RECORD *answers,
    struct RES_RECORD *auth, 
    struct RES_RECORD *addit);
