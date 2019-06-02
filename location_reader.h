#include <resolv.h>

/*
 * Transforma un RR de tipo LOC a un formato legible por los humanos.
 * Retorna un puntero con la dirección donde comienza el RR transformado.
 * *binary - Puntero que mantiene la referencia al RR que se va a transformar.
 * *ascii - Formato que tiene el RR de tipo LOC que se va a leer.
*/
const char *loc_ntoa(const unsigned char *binary, char *ascii) __THROW;
