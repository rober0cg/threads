
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "skl.h"

#define _CONFIG_C
#include "config.h"


int carga_config ( cfg_entrada **rpc )
{
	cfg_entrada *pc ;
	log_trace ( "carga_config" ) ;

// configuracion a capón para pruebas

#define MAX_HILOS_CAPON 3

	pc = (cfg_entrada *)malloc ( MAX_HILOS_CAPON * sizeof(cfg_entrada) ) ;
	if ( pc==(cfg_entrada *)0 ) {
		log_fatal ( "carga_config: malloc NULL" ) ;
		return -1 ;
	}

	pc[0].maxHilos = 5 ;
	strcpy ( pc[0].label,       "hilo0" ) ;
	strcpy ( pc[0].dirEntrada,  "./dat/ent0" ) ;
	strcpy ( pc[0].mskEntrada,  "*.dat" ) ;
	strcpy ( pc[0].dirSalida,   "./dat/sal0" ) ;
	strcpy ( pc[0].dirTratado,  "./dat/tra0" ) ;
	strcpy ( pc[0].fileMaestro, "./dat/mae0" ) ;

	pc[1].maxHilos = 5 ;
	strcpy ( pc[1].label,       "hilo1" ) ;
	strcpy ( pc[1].dirEntrada,  "./dat/ent1" ) ;
	strcpy ( pc[1].mskEntrada,  "*.dat" ) ;
	strcpy ( pc[1].dirSalida,   "./dat/sal1" ) ;
	strcpy ( pc[1].dirTratado,  "./dat/tra1" ) ;
	strcpy ( pc[1].fileMaestro, "./dat/mae1" ) ;

	pc[2].maxHilos = 5 ;
	strcpy ( pc[2].label,       "hilo2" ) ;
	strcpy ( pc[2].dirEntrada,  "./dat/ent2" ) ;
	strcpy ( pc[2].mskEntrada,  "*.dat" ) ;
	strcpy ( pc[2].dirSalida,   "./dat/sal2" ) ;
	strcpy ( pc[2].dirTratado,  "./dat/tra2" ) ;
	strcpy ( pc[2].fileMaestro, "./dat/mae2" ) ;

	*rpc = pc ;

	return MAX_HILOS_CAPON ;
}

int recarga_config ( cfg_entrada *pc )
{
	log_trace ( "carga_reconfig" ) ;

// configuracion a capón para pruebas
	pc[0].maxHilos = 4 ;

	pc[1].maxHilos = 3 ;

	pc[2].maxHilos = 2 ;

	return 0 ;
}

int libera_config ( cfg_entrada *pc )
{
	free ( (void *)pc ) ;
	return 0 ;
}




