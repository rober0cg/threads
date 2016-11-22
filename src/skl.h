#ifndef _SKL_H
#define _SKL_H

#include <limits.h>
#include <pthread.h>

// datos para pasar a cada hilo principal,
// hilo que se encargará del polling en el directorio,
// de arrancar hilos para cada fichero de entrada y controlar el máximo.

// datos para pasar a cada hilo encargado de procesar fichero
typedef struct st_th_fichero {
	pthread_t id ;
	int ret ;
	int fin ;
	struct st_th_entrada *pe ;
	char fileEntrada [NAME_MAX] ;
} th_fichero ;

#define MAX_FICHEROS_POR_ENTRADA 8
typedef struct st_th_entrada {
	pthread_t id ;
	int ret ;
	int fin ;
	int maxHilos ;
	int reconfMaxHilos ;
	pthread_mutex_t mutexNumHilos ;
	int numHilos ;
	char *label  ;
	char *dirEntrada   ;
	char *mskEntrada   ;
	char *dirSalida    ;
	char *dirTratado   ;
	char *fileMaestro  ;
	struct st_th_fichero hilos[MAX_FICHEROS_POR_ENTRADA] ;
} th_entrada ;

#ifndef _SKL_C

#endif // _SKL_C

#endif // _SKL_H
