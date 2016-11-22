#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define _LOG_C
#include "log.h"

static char *log_tag[] = {
	"FATAL",
	"ERROR",
	"WARN",
	"INFO",
	"DEBUG",
	"TRACE"
} ;


log_level_enum log_level = LOG_TRACE ;

static FILE *log_file = (FILE *)0 ;

void log_init ( char *params )
{
	log_level = LOG_WARN ;
	log_file = stderr ;
	return ;
}

void log_end ( )
{
	log_level = LOG_WARN ;
	log_file = stderr ;
	return ;
}


int log_print ( int level, const char *msg, va_list args )
{
	int lng=0 ;
	if ( log_level >= level ) {
		time_t now ;
		struct tm *ptm_info ;
		struct timeval tv ;
		char buffer [32] ;

		gettimeofday ( &tv, (struct timezone *)0 ) ;
		now = (time_t)tv.tv_sec ;
		ptm_info = localtime ( &now ) ;
		strftime ( buffer, 32, "%Y/%m/%d %H:%M:%S", ptm_info ) ;

		if ( log_file==(FILE *)0 )
			log_file = stderr ;

		{
			static pthread_mutex_t log_print_mutex = PTHREAD_MUTEX_INITIALIZER ;
			pthread_mutex_lock ( &log_print_mutex ) ;

			lng += fprintf ( log_file, "%s.%06d [%s] ", buffer, tv.tv_usec, log_tag[level] ) ;
			lng += vfprintf ( log_file, msg, args ) ;
			lng += fprintf ( log_file, "\n" ) ;

			pthread_mutex_unlock ( &log_print_mutex ) ;
		}

	}
	return lng ;
}

void log_fatal ( const char *msg, ... )
{
	va_list args ;
	va_start ( args, msg ) ;
	log_print ( LOG_FATAL, msg, args ) ;
	va_end ( args ) ;
	return ;
}

void log_error ( const char *msg, ... )
{
	va_list args ;
	va_start ( args, msg ) ;
	log_print ( LOG_ERROR, msg, args ) ;
	va_end ( args ) ;
	return ;
}

void log_warn  ( const char *msg, ... )
{
	va_list args ;
	va_start ( args, msg ) ;
	log_print ( LOG_WARN, msg, args ) ;
	va_end ( args ) ;
	return ;
}

void log_info  ( const char *msg, ... )
{
	va_list args ;
	va_start ( args, msg ) ;
	log_print ( LOG_INFO, msg, args ) ;
	va_end ( args ) ;
	return ;
}

void log_debug ( const char *msg, ... )
{
	va_list args ;
	va_start ( args, msg ) ;
	log_print ( LOG_DEBUG, msg, args ) ;
	va_end ( args ) ;
	return ;
}

void log_trace ( const char *msg, ... )
{
	va_list args ;
	va_start ( args, msg ) ;
	log_print ( LOG_TRACE, msg, args ) ;
	va_end ( args ) ;
	return ;
}

