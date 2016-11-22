#ifndef _CONFIG_H
#define _CONFIG_H

#include <limits.h>

typedef struct {
    int maxHilos ;
	char label       [NAME_MAX] ;
	char dirEntrada  [PATH_MAX] ;
	char mskEntrada  [NAME_MAX] ;
	char dirSalida   [PATH_MAX] ;
	char dirTratado  [PATH_MAX] ;
	char fileMaestro [PATH_MAX] ;
} cfg_entrada ;

#ifndef _CONFIG_C

extern int carga_config ( cfg_entrada **pc ) ;
extern int recarga_config ( cfg_entrada *pc ) ;
extern int libera_config ( cfg_entrada *pc ) ;

#endif // _CONFIG_C

#endif // _CONFIG_H
