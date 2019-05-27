#include "message_elements.h"

void handleResponse(struct DNS_HEADER *query_response_header, unsigned char *reader, unsigned char *response);
/*
void printResponse(
    struct DNS_HEADER *query_response_header, 
    struct RES_RECORD *answers,
    struct RES_RECORD *authority, 
    struct RES_RECORD *additional);
    */

//void printAnswers(int ans_count, struct RES_RECORD *answers);
//void printAuthority(int ns_count, struct RES_RECORD *auth);
