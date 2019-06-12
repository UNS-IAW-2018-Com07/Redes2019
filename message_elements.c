#include "message_elements.h"

void changeToQNameFormat(unsigned char* qname, char* hostname) 
{
    int host_length = strlen(hostname) + 2;
    char host_copy[host_length];
    strcpy(host_copy, hostname);
    strcat((char *) host_copy, ".");

    int qname_position = 0, hostname_position;
     
    for(hostname_position = 0; hostname_position < host_length; hostname_position++) 
    {
        if(host_copy[hostname_position] == '.') 
        {
            *qname = hostname_position - qname_position;
            qname++; 

            while(qname_position < hostname_position) 
            {
                *qname = host_copy[qname_position];
                qname++;
                qname_position++;
            }
            qname_position++; 
        }
    }
    *qname='\0';
}

void changeFromQNameFormatToNormalFormat(unsigned char* qname) 
{
    int qname_length =  (int) strlen((const char*) qname);
    int position_length = 0, i,j;

    if(qname_length == 0)
    {
        // consultaron por el root.    
        qname[0] = '\0'; 
        return; 
    }

    for(i = 0; i < qname_length; i++) 
    {
        position_length = qname[i];
        for(j = 0; j < (int)position_length; j++) 
        {
            qname[i] = qname[i+1];
            i++;
        }
        qname[i]='.';
    }
    qname[i-1] = '\0'; // removemos el ultimo punto innecesario
}
