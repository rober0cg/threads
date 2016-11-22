
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>
#include <dirent.h>
#include <fnmatch.h>

#include "log.h"
#include "skl.h"
#include "config.h"
#include "proceso.h"


static volatile int sigTermRcvd = 0 ;
static void sigterm ( int sig ) { sigTermRcvd = -1 ; }

static volatile int sigHupRcvd = 0 ;
static void sighup ( int sig ) { sigHupRcvd = -1 ; }

static volatile int sigUsr1Rcvd = 0 ;
static void sigusr1 ( int sig ) { sigUsr1Rcvd = -1 ; }

static int dump_threads ( int num, th_entrada *cntxt ) ;
static int hilo_entrada ( th_entrada *cntxt ) ;
static int hilo_fichero ( th_fichero *cntxt ) ;


int main ( int argc, char *argv[] )
{
	int i, rc ;
	cfg_entrada *cfg = (cfg_entrada *)0 ;
	th_entrada *thEntrada = (th_entrada *)0 ;
	int numHilos = 0 ;

	log_level = LOG_INFO ;

	numHilos = carga_config ( &cfg ) ;
	if ( numHilos<0 ) {
		log_fatal ( "main: carga_config error" ) ;
		return 1 ;
	}

	rc = carga_memoria ( cfg ) ;
	if ( rc<0 ) {
		log_fatal ( "main: carga_memoria error" ) ;
		return 2 ;
	}

	thEntrada = (th_entrada *)malloc ( sizeof(th_entrada) * numHilos ) ;
	if ( thEntrada == (th_entrada *)0 ) {
		log_fatal ( "main: malloc error" ) ;
		return 3 ;
	}


// arranque de los hilos principales
	for ( i=0 ; i<numHilos ; i++ ) {
	// copiamos configuración a estructura de contexto del hilo
		thEntrada[i].label       = cfg[i].label      ;
		thEntrada[i].dirEntrada  = cfg[i].dirEntrada ;
		thEntrada[i].mskEntrada  = cfg[i].mskEntrada ;
		thEntrada[i].dirSalida   = cfg[i].dirSalida  ;
		thEntrada[i].dirTratado  = cfg[i].dirTratado ;
		thEntrada[i].fileMaestro = cfg[i].fileMaestro;
		thEntrada[i].maxHilos    = cfg[i].maxHilos   ;
		thEntrada[i].reconfMaxHilos = 0 ;

		log_info ( "main: hilo '%s' creando...", thEntrada[i].label ) ;

		thEntrada[i].ret = 0 ;
		thEntrada[i].fin = 0 ;
		rc = pthread_create (
				&thEntrada[i].id,
				(pthread_attr_t *)0,
				(void *(* )(void *))hilo_entrada,
				(void *)&(thEntrada[i])
			 ) ;
		if ( rc<0 ) {
			log_fatal ( "main: error pthread_create '%s' -> %d - %s", thEntrada[i].label,
				errno, strerror(errno)
			) ;
		}
		else {
			log_info ( "main: hilo '%s' creado OK", thEntrada[i].label ) ;
		}

	}

// el proceso padre espera eventos de finalización o reconfiguración
	signal ( SIGTERM, sigterm ) ;
	signal ( SIGHUP , sighup  ) ;
	signal ( SIGUSR1, sigusr1 ) ;

	while ( !sigTermRcvd ) {
		sleep(10) ;
		if ( sigHupRcvd ) {
			sigHupRcvd = 0 ;
			log_info ( "main: SIGHUP rcvd" ) ;
			recarga_config ( cfg ) ;
			for ( i=0 ; i<numHilos ; i++ ) {
				thEntrada[i].reconfMaxHilos = cfg[i].maxHilos ;
			}
		}
		if ( sigUsr1Rcvd ) {
			sigUsr1Rcvd = 0 ;
			dump_threads ( numHilos, thEntrada ) ;
		}
		if ( sigTermRcvd ) {
			log_info ( "main: SIGTERM rcvd" ) ;
		}
	}

    signal ( SIGUSR1, SIG_DFL ) ;
    signal ( SIGHUP , SIG_DFL ) ;
	signal ( SIGTERM, SIG_DFL ) ;


// salida ordenada
// espera de finalización de los hijos principales
	for ( i=0 ; i<numHilos ; i++ ) {
		log_trace ( "main: hilo '%s' avisando para finalizar",
			thEntrada[i].label
		) ;
		thEntrada[i].fin = -1 ;
	}

	for ( i=0 ; i<numHilos ; i++ ) {
		log_info ( "main: hilo '%s' esperando finalizar...", thEntrada[i].label ) ;
		rc = pthread_join (
				thEntrada[i].id,
				(void **)0
			 ) ;
		if ( rc<0 ) {
			log_fatal ( "main: error pthread_hoin '%s' -> %d - %s", thEntrada[i].label,
				errno, strerror(errno)
			) ;
		}
		else {
			log_info ( "main: hilo '%s' finalizado ret %d", thEntrada[i].label,
				thEntrada[i].ret
			) ;
		}
	}


	libera_memoria ( ) ;

	libera_config ( cfg ) ;

	return 0 ;
}



static int hilo_entrada ( th_entrada *pc )
{
    int i, j, rc ;

	log_info ( "hilo_entrada %s INI", pc->label ) ;

	log_debug ( "hilo_entrada %s -> dirEntrada  = '%s'", pc->label, pc->dirEntrada ) ;
	log_debug ( "hilo_entrada %s -> mskEntrada  = '%s'", pc->label, pc->mskEntrada ) ;
	log_debug ( "hilo_entrada %s -> dirSalida   = '%s'", pc->label, pc->dirSalida  ) ;
	log_debug ( "hilo_entrada %s -> dirTratado  = '%s'", pc->label, pc->dirTratado ) ;
	log_debug ( "hilo_entrada %s -> fileMaestro = '%s'", pc->label, pc->fileMaestro ) ;
	log_debug ( "hilo_entrada %s -> maxHilos = '%d'", pc->label, pc->maxHilos ) ;

	for ( j=0 ; j<MAX_FICHEROS_POR_ENTRADA ; j++ ) {
		pc->hilos[j].ret = 0 ;
		pc->hilos[j].fin = -1 ;
		pc->hilos[j].pe = pc ;
		pc->hilos[j].fileEntrada[0] = (char)0 ;
	}

// inicializamos contador de hilos y su mutex
	pc->numHilos = 0 ;
//	pc->mutexNumHilos = PTHREAD_MUTEX_INITIALIZER ;
	pthread_mutex_init ( &pc->mutexNumHilos, 0 ) ;


	while ( !pc->fin ) { // mientras que el proceso padre no indique finalizar
//		log_trace ( "hilo_entrada %s: running", pc->label ) ;

// listar ficheros, hasta un máximo de pteHilos
		int pteHilos = pc->maxHilos - pc->numHilos ;
		if ( pteHilos > 0 ) { // si podemos arrancar buscamos
			DIR *dir = opendir ( pc->dirEntrada ) ;
			if ( dir!=(DIR *)0 ) {
				struct dirent *de ;
				i=0 ;
				while ( i<pteHilos && (de=readdir(dir))!=(struct dirent *)0 ) {
					if ( fnmatch ( pc->mskEntrada, de->d_name, 0 ) )
						continue ;
					i++;

// hijos que procesan ficheros en modo detach (sin join)
					log_trace ( "hilo_entrada %s: identificado fichero '%s'", pc->label,
						de->d_name 
					) ;

// localizamos entrada libre (ret==-1) en el array de hilos
					for ( j=0 ; j<MAX_FICHEROS_POR_ENTRADA ; j++ ) {
						if ( pc->hilos[j].fin == -1 )
							break ;
					}
					if ( j<MAX_FICHEROS_POR_ENTRADA ) { // hilo libre pc->hilos[j]

// renombramos fichero con extension temporal de trabajo
						char fnam[PATH_MAX] ;
						strcpy ( fnam, pc->dirEntrada ) ;
						strcat ( fnam, "/" ) ;
						strcat ( fnam, de->d_name ) ;

						char fwrk[PATH_MAX] ;
						strcpy ( fwrk, fnam ) ;
						strcat ( fwrk, "-wrk" ) ;

						log_trace ( "hilo_entrada '%s': rename '%s' -> '%s'", pc->label,
							fnam, fwrk
						) ;
						rc = rename ( fnam, fwrk ) ;
						if ( rc<0 ) {
							log_error ( "hilo_entrada '%s': rename error -> %d - %s", pc->label,
								errno, strerror(errno)
							) ;
						}

						log_info ( "hilo_entrada '%s': creando hilo para '%s'", pc->label,
							de->d_name
						) ;

// incremento del número de hilos
						pthread_mutex_lock ( &pc->mutexNumHilos ) ;
						pc->numHilos ++ ;
						pc->hilos[j].fin = 0 ;
						pthread_mutex_unlock ( &pc->mutexNumHilos ) ;
						pc->hilos[j].ret = 0 ;
						pc->hilos[j].pe = pc ;
						strcpy ( pc->hilos[j].fileEntrada, de->d_name ) ;
						strcat (pc->hilos[j].fileEntrada, "-wrk" ) ;

						rc = pthread_create (
								&(pc->hilos[j].id),
								(pthread_attr_t *)0,
								(void *(* )(void *))hilo_fichero,
								(void *)&(pc->hilos[j])
						) ;
						if ( rc<0 ) {
							log_fatal ( "hilo_entrada %s: error pthread_create '%s' -> %d - %s",
								pc->label,
								errno, strerror(errno)
							) ;
						}
						else {
							rc = pthread_detach ( pc->hilos[j].id ) ;
							log_info ( "hilo_entrada %s: hilo_fichero '%s' creado OK detach %d",
								pc->label, pc->hilos[i].fileEntrada, rc
							) ;
						}
					}
					else {
						log_fatal ( "hilo_entrada %s: sin slots para hilos", pc->label ) ;
					}
				}
				closedir ( dir ) ;
			}
			else {
				log_fatal ( "hilo_entrada %s: error opendir '%s'", pc->label, pc->dirEntrada ) ;
			}
		}
		else {
			log_trace ( "hilo_entrada %s: maxHilos arrancados %d", pc->label, pc->numHilos ) ;
		}

		usleep ( 100000 ) ; // cada décima de segundo

		if ( pc->reconfMaxHilos>0 ) {
			log_debug ( "hilo_entrada %s reconfMaxHilos %d", pc->label, pc->reconfMaxHilos ) ;
			pc->maxHilos = pc->reconfMaxHilos ;
			pc->reconfMaxHilos = 0 ;
		}
		if ( pc->fin ) {
			log_trace ( "hilo_entrada %s recibido aviso de fin", pc->label ) ;

// esperar a los posibles hijos que finalicen
			while ( pc->numHilos > 0 ) {
				log_trace ( "hilo_entrada %s esperando %d hilos", pc->label, pc->numHilos ) ;
				usleep ( 100000 ) ; // una décima
			}

		}

	}






	log_info ( "hilo_entrada %s FIN", pc->label ) ;

	pc->ret = 0 ;
	pthread_exit ( (void *)&pc->ret ) ;

	return 0 ;
}


static int hilo_fichero ( th_fichero *pc )
{
	int rc=0 ;

	log_info ( "hilo_fichero %s INI:  %s", pc->pe->label, pc->fileEntrada ) ;

// preparamos para hacer el rename tras el proceso
	char fwrk[PATH_MAX] ;
	char fdst[PATH_MAX] ;
	strcpy ( fwrk, pc->pe->dirEntrada ) ;
	strcat ( fwrk, "/" ) ;
	strcat ( fwrk, pc->fileEntrada ) ;

	rc = proceso ( pc->pe->label, pc->fileEntrada ) ;
	pc->ret = rc ;

	if ( rc<0 ) { // error, renombramos el fichero como err
		strcpy ( fdst, fwrk ) ;
		char *pwrk = strstr ( fdst, "-wrk" ) ;
		if ( pwrk!= (char *)0 ) {
			*pwrk = (char)0 ;
		}
		strcat ( fdst, "-err" ) ;
	}
	else { // ok, movemos fichero a Tratado
		char ftra[PATH_MAX] ;
		strcpy ( fdst, pc->pe->dirTratado ) ;
		strcat ( fdst, "/" ) ;
		strcat ( fdst, pc->fileEntrada ) ;
		char *pwrk = strstr ( fdst, "-wrk" ) ;
		if ( pwrk!= (char *)0 ) {
			*pwrk = (char)0 ;
		}
	}

	log_trace ( "hilo_fichero '%s': rename '%s' -> '%s'", pc->pe->label, fwrk, fdst ) ;
	rc = rename ( fwrk, fdst ) ;
	if ( rc<0 ) {
		log_error ( "hilo_fichero '%s': rename error -> %d - %s", pc->pe->label,
			errno, strerror(errno)
		) ;
	}

	pthread_mutex_lock ( &(pc->pe->mutexNumHilos) ) ;
	pc->pe->numHilos -- ;
	pc->fin = -1 ;
	pthread_mutex_unlock ( &(pc->pe->mutexNumHilos) ) ;
	
	log_info ( "hilo_fichero %s FIN:  %s", pc->pe->label, pc->fileEntrada ) ;

	pthread_exit ( (void *)&pc->ret ) ;

	return 0 ;
}


static int dump_threads ( int num, th_entrada *ent )
{
	int i, j;

	for ( i=0 ; i<num ; i++ ) {
		fprintf ( stdout, "entrada %s {\n", ent[i].label ) ;
		fprintf ( stdout, "\tmaxHilos = %d\n", ent[i].maxHilos ) ;
		fprintf ( stdout, "\tnumHilos = %d\n", ent[i].numHilos ) ;
		for ( j=0 ; j<MAX_FICHEROS_POR_ENTRADA ; j++ ) {
			th_fichero *fic = &ent[i].hilos[j] ;
			if ( fic->fin<0 ) // hueco disponible
				continue ;
			if ( fic->fin>=0 ) { // con hilo corriendo
				fprintf ( stdout, "\thilo %d - fileEntrada = %s\n", j, fic->fileEntrada ) ;
			}
		}
		fprintf ( stdout, "}\n" ) ;
	}

	return 0 ;
}
