
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define _PROCESO_C
#include "proceso.h"

#include "config.h"
#include "log.h"


int carga_memoria ( cfg_entrada *pCfg )
{

	return 0 ;
}

int libera_memoria ( )
{
	return 0 ;
}



int proceso ( char *lbl, char *fEnt )
{
	log_trace ( "proceso '%s' INI: fileEntrada '%s'", lbl, fEnt ) ;

	static int primeravez = -1 ;
	if ( primeravez ) {
		primeravez = 0 ;
		srand ( time ( (time_t *)0 ) ) ;
	}

	usleep ( 1+(int)(5000000.0*rand()/(RAND_MAX+1.0)) ) ;

	log_trace ( "proceso '%s' FIN: fileEntrada '%s'", lbl, fEnt ) ;
	return 0 ;
}

