#ifndef _PROCESO_H
#define _PROCESO_H

#include "config.h"

#ifndef _PROCESO_C

extern int carga_memoria ( cfg_entrada *pCfg ) ;
extern int libera_memoria ( void ) ;

extern int proceso ( char *label, char *fileEntrada ) ;

#endif // _PROCESO_C

#endif // _PROCESO_H

