#include <resolv.h>

/* takes an XeY precision/size value, returns a string representation. */
const char *precsize_ntoa(u_int8_t prec);

/* takes an on-the-wire LOC RR and formats it in a human readable format. */
const char *loc_ntoa(const unsigned char *binary, char *ascii) __THROW;
