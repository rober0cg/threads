#ifndef _LOG_H
#define _LOG_H

typedef enum {
	LOG_FATAL = 0,
	LOG_ERROR = 1,
	LOG_WARN = 2,
	LOG_INFO = 3,
	LOG_DEBUG = 4,
	LOG_TRACE = 5
} log_level_enum ;

#ifndef _LOG_C

extern log_level_enum log_level ;

extern void log_init  ( char *params ) ;
extern void log_end   ( ) ;

extern void log_print ( log_level_enum level, const char *msg, ... ) ;

extern void log_fatal ( const char* msg, ... ) ;
extern void log_error ( const char* msg, ... ) ;
extern void log_warn  ( const char* msg, ... ) ;
extern void log_info  ( const char* msg, ... ) ;
extern void log_debug ( const char* msg, ... ) ;
extern void log_trace ( const char* msg, ... ) ;

#endif // _LOG_C

#endif // _LOG_H
